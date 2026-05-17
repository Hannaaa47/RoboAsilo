package com.example.blue_botones;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

public class ControlFragment extends Fragment implements MainActivity.BleListener {

    private TextView tvEstadoCtrl;
    private MainActivity mainActivity;

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_control, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mainActivity = (MainActivity) requireActivity();
        mainActivity.setBleListener(this);

        tvEstadoCtrl = view.findViewById(R.id.tv_estado_ctrl);

        // Sincronizar indicador de estado al entrar a la vista
        if (mainActivity.conectado) {
            setEstado("● Conectado", "#69F0AE");
        } else {
            setEstado("● Desconectado", "#FF5252");
        }

        // ── Botones de control ────────────────────────────────────────────────
        // Cada botón llama a enviarCmd con la letra correspondiente

        view.findViewById(R.id.btn_arriba).setOnClickListener(v -> enviarCmd("F"));
        view.findViewById(R.id.btn_abajo).setOnClickListener(v -> enviarCmd("B"));
        view.findViewById(R.id.btn_izquierda).setOnClickListener(v -> enviarCmd("L"));
        view.findViewById(R.id.btn_derecha).setOnClickListener(v -> enviarCmd("R"));
        view.findViewById(R.id.btn_sierra_on).setOnClickListener(v -> enviarCmd("X"));
        view.findViewById(R.id.btn_sierra_off).setOnClickListener(v -> enviarCmd("O"));
        view.findViewById(R.id.btn_stop).setOnClickListener(v -> enviarCmd("S"));
    }

    /** Envía un comando al Arduino y muestra feedback visual */
    private void enviarCmd(String cmd) {
        if (!mainActivity.conectado) {
            Toast.makeText(requireContext(), "Conecta un dispositivo primero", Toast.LENGTH_SHORT).show();
            return;
        }
        boolean ok = mainActivity.enviar(cmd);
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mainActivity != null) mainActivity.setBleListener(this);
    }

    @Override
    public void onEstadoCambiado(String texto, String color) {
        setEstado(texto, color);
    }

    @Override
    public void onMensajeRecibido(String mensaje) {
        // En esta vista no mostramos el log completo, solo actualizamos estado si es necesario
    }

    private void setEstado(String texto, String color) {
        if (tvEstadoCtrl == null) return;
        tvEstadoCtrl.setText(texto);
        tvEstadoCtrl.setTextColor(android.graphics.Color.parseColor(color));
    }
}