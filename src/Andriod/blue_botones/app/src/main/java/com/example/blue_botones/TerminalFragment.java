package com.example.blue_botones;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ScrollView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

public class TerminalFragment extends Fragment implements MainActivity.BleListener {

    private TextView tvEstado, tvMensajes;
    private ScrollView scrollMensajes;
    private Button btnBuscar, btnDesconectar, btnEnviar;
    private EditText etMensaje;
    private ListView listDispositivos;
    private View layoutConectado;

    private MainActivity mainActivity;

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_terminal, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mainActivity = (MainActivity) requireActivity();
        mainActivity.setBleListener(this);

        tvEstado        = view.findViewById(R.id.tv_estado);
        tvMensajes      = view.findViewById(R.id.tv_mensajes);
        scrollMensajes  = view.findViewById(R.id.scroll_mensajes);
        btnBuscar       = view.findViewById(R.id.btn_buscar);
        btnDesconectar  = view.findViewById(R.id.btn_desconectar);
        btnEnviar       = view.findViewById(R.id.btn_enviar);
        etMensaje       = view.findViewById(R.id.et_mensaje);
        listDispositivos = view.findViewById(R.id.list_dispositivos);
        layoutConectado = view.findViewById(R.id.layout_conectado);

        // Asignar adapter de dispositivos del MainActivity
        listDispositivos.setAdapter(mainActivity.deviceAdapter);

        // Sincronizar estado visual según si está conectado
        if (mainActivity.conectado) {
            listDispositivos.setVisibility(View.GONE);
            layoutConectado.setVisibility(View.VISIBLE);
        } else {
            listDispositivos.setVisibility(View.VISIBLE);
            layoutConectado.setVisibility(View.GONE);
        }

        // ── Listeners ────────────────────────────────────────────────────────

        btnBuscar.setOnClickListener(v -> mainActivity.iniciarEscaneo());

        listDispositivos.setOnItemClickListener((adapterView, v, i, l) -> {
            mainActivity.detenerEscaneo();
            mainActivity.conectar(mainActivity.listaDevices.get(i));
        });

        btnEnviar.setOnClickListener(v -> {
            String texto = etMensaje.getText().toString().trim();
            if (!texto.isEmpty()) {
                boolean ok = mainActivity.enviar(texto);
                if (ok) {
                    agregarMensaje(">> " + texto);
                    etMensaje.setText("");
                }
            }
        });

        btnDesconectar.setOnClickListener(v -> {
            mainActivity.desconectar();
            listDispositivos.setVisibility(View.VISIBLE);
            layoutConectado.setVisibility(View.GONE);
        });
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mainActivity != null) mainActivity.setBleListener(this);
    }

    @Override
    public void onEstadoCambiado(String texto, String color) {
        if (tvEstado == null) return;
        tvEstado.setText(texto);
        tvEstado.setTextColor(android.graphics.Color.parseColor(color));

        // Mostrar/ocultar vistas según estado
        boolean conectado = texto.startsWith("● Conectado");
        boolean desconectado = texto.startsWith("● Desconectado");

        if (conectado) {
            listDispositivos.setVisibility(View.GONE);
            layoutConectado.setVisibility(View.VISIBLE);
        } else if (desconectado) {
            listDispositivos.setVisibility(View.VISIBLE);
            layoutConectado.setVisibility(View.GONE);
        }
    }

    @Override
    public void onMensajeRecibido(String mensaje) {
        agregarMensaje(mensaje);
    }

    private void agregarMensaje(String mensaje) {
        if (tvMensajes == null) return;
        String actual = tvMensajes.getText().toString();
        tvMensajes.setText(actual + mensaje + "\n");
        scrollMensajes.post(() -> scrollMensajes.fullScroll(ScrollView.FOCUS_DOWN));
    }
}