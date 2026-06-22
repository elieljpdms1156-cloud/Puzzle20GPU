package com.puzzle20.gpu;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.*;
import org.bitcoinj.core.ECKey;
import java.security.MessageDigest;

public class MainActivity extends Activity {
    private TextView status, result, speed;
    private ProgressBar bar;
    private Button btn;
    private Handler h;
    private volatile boolean running;
    private GPUEngine gpu;

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        setContentView(R.layout.activity_main);
        
        status = findViewById(R.id.tvStatus);
        result = findViewById(R.id.tvResult);
        speed = findViewById(R.id.tvSpeed);
        bar = findViewById(R.id.progressBar);
        btn = findViewById(R.id.btnStart);
        h = new Handler(Looper.getMainLooper());
        
        gpu = new GPUEngine();
        boolean hasGPU = gpu.initGPU();
        
        status.setText("🎯 Puzzle #20\n" +
                      (hasGPU ? "⚡ GPU Adreno ATIVA!" : "🔵 CPU mode") +
                      "\n0x80000-0xFFFFF\n524288 keys");
        speed.setText("Ready");
        
        btn.setOnClickListener(v -> start());
    }

    private void start() {
        if (running) return;
        running = true;
        btn.setEnabled(false);
        btn.setText("SEARCHING...");
        result.setText("");
        
        new Thread(() -> {
            try {
                MessageDigest sha = MessageDigest.getInstance("SHA-256");
                MessageDigest ripe = MessageDigest.getInstance("RIPEMD160");
                byte[] target = hexToBytes("b907c3a2a3b27789dfb509b730dd47703c272868");
                
                long startTime = System.currentTimeMillis();
                long checked = 0;
                
                for (int k = 0x80000; k <= 0xFFFFF && running; k++) {
                    byte[] priv = new byte[32];
                    priv[29] = (byte)(k >> 16);
                    priv[30] = (byte)(k >> 8);
                    priv[31] = (byte)k;
                    
                    ECKey ec = ECKey.fromPrivate(priv, true);
                    byte[] hash = ec.getPubKeyHash();
                    
                    if (java.util.Arrays.equals(hash, target)) {
                        final String keyHex = Integer.toHexString(k);
                        long elapsed = System.currentTimeMillis() - startTime;
                        double spd = (checked * 1000.0) / Math.max(elapsed, 1);
                        
                        h.post(() -> {
                            result.setText("🎉 FOUND!\nKey: " + keyHex);
                            status.setText("✅ SOLVED!");
                            speed.setText(String.format("%,.0f k/s", spd));
                            btn.setText("START");
                            btn.setEnabled(true);
                        });
                        running = false;
                        return;
                    }
                    
                    checked++;
                    if (checked % 1000 == 0) {
                        final long c = checked;
                        final int k2 = k;
                        long elapsed = System.currentTimeMillis() - startTime;
                        final double spd = (c * 1000.0) / Math.max(elapsed, 1);
                        final int pct = (int)((c * 100) / 524288);
                        
                        h.post(() -> {
                            status.setText("Key: " + Integer.toHexString(k2) + 
                                         "\n" + c + "/524288\n" + pct + "%");
                            speed.setText(String.format("%,.0f keys/s", spd));
                            bar.setProgress((int)c);
                        });
                    }
                }
                
                h.post(() -> {
                    status.setText("Finished!");
                    btn.setText("START");
                    btn.setEnabled(true);
                });
                running = false;
            } catch (Exception e) {
                h.post(() -> status.setText("Error: " + e.getMessage()));
                running = false;
            }
        }).start();
    }

    private byte[] hexToBytes(String s) {
        byte[] d = new byte[s.length()/2];
        for (int i = 0; i < d.length; i++)
            d[i] = (byte)Integer.parseInt(s.substring(i*2, i*2+2), 16);
        return d;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        running = false;
        if (gpu != null) gpu.releaseGPU();
    }
}
