package com.example.blue_botones;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.ArrayAdapter;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import com.google.android.material.bottomnavigation.BottomNavigationView;

import java.util.ArrayList;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    // UUID del servicio serial HM-10 / módulos BLE genéricos
    public static final UUID UUID_SERVICE        = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb");
    public static final UUID UUID_CHARACTERISTIC = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb");

    private static final long SCAN_TIMEOUT = 10_000;

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothLeScanner bleScanner;
    private BluetoothGatt bluetoothGatt;
    private BluetoothGattCharacteristic txRxCharacteristic;

    public final ArrayList<BluetoothDevice> listaDevices = new ArrayList<>();
    public final ArrayList<String> listaNombres = new ArrayList<>();
    public ArrayAdapter<String> deviceAdapter;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private boolean escaneando = false;
    public boolean conectado = false;

    // Listeners para notificar a los fragmentos
    public interface BleListener {
        void onEstadoCambiado(String texto, String color);
        void onMensajeRecibido(String mensaje);
    }

    private BleListener bleListener;

    public void setBleListener(BleListener listener) {
        this.bleListener = listener;
    }

    // ── Launchers ──────────────────────────────────────────────────────────────

    private final ActivityResultLauncher<Intent> btEnableLauncher =
            registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
                if (bluetoothAdapter != null && bluetoothAdapter.isEnabled()) {
                    pedirPermisos();
                } else {
                    Toast.makeText(this, "Activa el Bluetooth para continuar", Toast.LENGTH_SHORT).show();
                }
            });

    private final ActivityResultLauncher<String[]> permisosLauncher =
            registerForActivityResult(new ActivityResultContracts.RequestMultiplePermissions(), result -> {
                boolean todosOk = true;
                for (Boolean granted : result.values()) {
                    if (!granted) { todosOk = false; break; }
                }
                if (todosOk) {
                    Toast.makeText(this, "Permisos concedidos. Pulsa 'Buscar'", Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(this, "Se necesitan permisos de Bluetooth", Toast.LENGTH_LONG).show();
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        BluetoothManager btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = btManager.getAdapter();

        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Este dispositivo no soporta Bluetooth", Toast.LENGTH_LONG).show();
            return;
        }

        deviceAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, listaNombres);

        if (!bluetoothAdapter.isEnabled()) {
            btEnableLauncher.launch(new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE));
        } else {
            pedirPermisos();
        }

        // ── Bottom Navigation ───────────────────────────────────────────────
        BottomNavigationView nav = findViewById(R.id.bottom_nav);

        // Fragmento inicial: Terminal
        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.fragment_container, new TerminalFragment())
                    .commit();
        }

        nav.setOnItemSelectedListener(item -> {
            Fragment frag;
            int id = item.getItemId();
            if (id == R.id.nav_terminal) {
                frag = new TerminalFragment();
            } else if (id == R.id.nav_control) {
                frag = new ControlFragment();
            } else {
                return false;
            }
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.fragment_container, frag)
                    .commit();
            return true;
        });
    }

    // ── Permisos ───────────────────────────────────────────────────────────────

    private void pedirPermisos() {
        String[] permisos = {
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
        };
        boolean faltaAlguno = false;
        for (String p : permisos) {
            if (ContextCompat.checkSelfPermission(this, p) != PackageManager.PERMISSION_GRANTED) {
                faltaAlguno = true;
                break;
            }
        }
        if (faltaAlguno) permisosLauncher.launch(permisos);
    }

    // ── Escaneo BLE ───────────────────────────────────────────────────────────

    @SuppressWarnings("MissingPermission")
    public void iniciarEscaneo() {
        if (escaneando) return;

        listaDevices.clear();
        listaNombres.clear();
        deviceAdapter.notifyDataSetChanged();
        notificarEstado("Buscando dispositivos BLE...", "#FFD54F");

        bleScanner = bluetoothAdapter.getBluetoothLeScanner();
        if (bleScanner == null) {
            Toast.makeText(this, "El escáner BLE no está disponible", Toast.LENGTH_SHORT).show();
            return;
        }

        escaneando = true;
        bleScanner.startScan(scanCallback);

        handler.postDelayed(() -> {
            detenerEscaneo();
            if (listaDevices.isEmpty()) {
                notificarEstado("No se encontraron dispositivos", "#FF5252");
            }
        }, SCAN_TIMEOUT);
    }

    @SuppressWarnings("MissingPermission")
    public void detenerEscaneo() {
        if (escaneando && bleScanner != null) {
            bleScanner.stopScan(scanCallback);
            escaneando = false;
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @SuppressWarnings("MissingPermission")
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            if (!listaDevices.contains(device)) {
                listaDevices.add(device);
                String nombre = device.getName() != null ? device.getName() : "Desconocido";
                listaNombres.add(nombre + "\n" + device.getAddress());
                deviceAdapter.notifyDataSetChanged();
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            handler.post(() -> {
                notificarEstado("Error al escanear: " + errorCode, "#FF5252");
                escaneando = false;
            });
        }
    };

    // ── Conexión BLE ──────────────────────────────────────────────────────────

    @SuppressWarnings("MissingPermission")
    public void conectar(BluetoothDevice device) {
        notificarEstado("Conectando...", "#FFD54F");
        bluetoothGatt = device.connectGatt(this, false, gattCallback);
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        @SuppressWarnings("MissingPermission")
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                conectado = true;
                gatt.discoverServices();
                handler.post(() -> notificarEstado("Conectando, iniciando servicios...", "#FFD54F"));

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                conectado = false;
                txRxCharacteristic = null;
                handler.post(() -> {
                    notificarEstado("● Desconectado", "#FF5252");
                    Toast.makeText(MainActivity.this, "Desconectado", Toast.LENGTH_SHORT).show();
                });
            }
        }

        @SuppressWarnings("MissingPermission")
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                handler.post(() -> Toast.makeText(MainActivity.this,
                        "Error al descubrir servicios", Toast.LENGTH_SHORT).show());
                return;
            }

            BluetoothGattService service = gatt.getService(UUID_SERVICE);
            if (service != null) {
                txRxCharacteristic = service.getCharacteristic(UUID_CHARACTERISTIC);
                if (txRxCharacteristic != null) {
                    int props = txRxCharacteristic.getProperties();
                    String propStr = "Props: "
                            + ((props & BluetoothGattCharacteristic.PROPERTY_READ)             != 0 ? "READ " : "")
                            + ((props & BluetoothGattCharacteristic.PROPERTY_WRITE)            != 0 ? "WRITE " : "")
                            + ((props & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)!= 0 ? "WRITE_NR " : "")
                            + ((props & BluetoothGattCharacteristic.PROPERTY_NOTIFY)           != 0 ? "NOTIFY" : "");
                    handler.post(() -> {
                        if (bleListener != null) bleListener.onMensajeRecibido("ℹ " + propStr);
                    });
                    gatt.setCharacteristicNotification(txRxCharacteristic, true);
                    handler.post(() -> notificarEstado("● Conectado", "#69F0AE"));
                } else {
                    handler.post(() -> Toast.makeText(MainActivity.this,
                            "Característica FFE1 no encontrada", Toast.LENGTH_LONG).show());
                }
            } else {
                handler.post(() -> Toast.makeText(MainActivity.this,
                        "Servicio FFE0 no encontrado", Toast.LENGTH_LONG).show());
            }
        }

        @Override
        public void onCharacteristicChanged(@NonNull BluetoothGatt gatt,
                                            @NonNull BluetoothGattCharacteristic characteristic,
                                            @NonNull byte[] value) {
            String dato = new String(value);
            handler.post(() -> {
                if (bleListener != null) bleListener.onMensajeRecibido("<< " + dato.trim());
            });
        }

        @Override
        @SuppressWarnings("deprecation")
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            String dato = new String(characteristic.getValue());
            handler.post(() -> {
                if (bleListener != null) bleListener.onMensajeRecibido("<< " + dato.trim());
            });
        }
    };

    // ── Envío BLE ─────────────────────────────────────────────────────────────

    @SuppressWarnings({"MissingPermission", "deprecation"})
    public boolean enviar(String texto) {
        if (!conectado || bluetoothGatt == null || txRxCharacteristic == null) {
            Toast.makeText(this, "No hay conexión activa", Toast.LENGTH_SHORT).show();
            return false;
        }

        byte[] datos = texto.getBytes();
        int propiedades = txRxCharacteristic.getProperties();
        boolean soportaWriteNoResp = (propiedades & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0;
        boolean soportaWrite       = (propiedades & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0;

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.TIRAMISU) {
            int writeType = soportaWriteNoResp
                    ? BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                    : BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT;
            int resultado = bluetoothGatt.writeCharacteristic(txRxCharacteristic, datos, writeType);
            return resultado == BluetoothGatt.GATT_SUCCESS;
        } else {
            txRxCharacteristic.setValue(datos);
            if (soportaWriteNoResp) {
                txRxCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE);
            } else if (soportaWrite) {
                txRxCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            }
            return bluetoothGatt.writeCharacteristic(txRxCharacteristic);
        }
    }

    // ── Desconexión ───────────────────────────────────────────────────────────

    @SuppressWarnings("MissingPermission")
    public void desconectar() {
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
        conectado = false;
        txRxCharacteristic = null;
        notificarEstado("● Desconectado", "#FF5252");
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    private void notificarEstado(String texto, String color) {
        if (bleListener != null) bleListener.onEstadoCambiado(texto, color);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        detenerEscaneo();
        desconectar();
    }
}