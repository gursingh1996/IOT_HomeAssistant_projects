package com.example.homelock;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class MainActivity extends AppCompatActivity {
    TextView status;
    Button btn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        init();
    }

    private void init(){
        status = findViewById(R.id.status);
        btn = findViewById(R.id.btn);
        getStatusRequest();         //pins the door lock esp to check if it is online
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendUnlockRequest();
            }
        });
    }

    private void sendUnlockRequest(){
        OkHttpClient client = new OkHttpClient();
        String url = "http://192.168.0.176/lockopen";
        Request request = new Request.Builder().url(url).build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(@NonNull Call call, @NonNull IOException e) {
                e.printStackTrace();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        status.setText("offline");
                    }
                });
            }

            @Override
            public void onResponse(@NonNull Call call, @NonNull Response response) throws IOException {
                if(response.isSuccessful()){
                    Log.e("Response", response.body().string());
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            status.setText("online");
                        }
                    });
                }
            }
        });
    }

    private void getStatusRequest(){
        OkHttpClient client = new OkHttpClient();
        String url = "http://192.168.0.176/status";
        Request request = new Request.Builder().url(url).build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(@NonNull Call call, @NonNull IOException e) {
                e.printStackTrace();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        status.setText("offline");
                    }
                });
            }

            @Override
            public void onResponse(@NonNull Call call, @NonNull Response response) throws IOException {
                if(response.isSuccessful()){
                    Log.e("Response", response.body().string());
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            status.setText("online");
                        }
                    });
                }
            }
        });
    }
}