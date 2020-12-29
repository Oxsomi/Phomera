package net.osomi.phomera;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import android.widget.TextView;

import org.freedesktop.gstreamer.GStreamer;

import java.io.InvalidObjectException;

public class MainActivity extends AppCompatActivity {

    private static boolean isInit = false, canInit = false, isReady = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        if(!isInit)
            try {

                if(!canInit)
                    throw new InvalidObjectException("initNative failed");

                GStreamer.init(this);
                isInit = true;
            }
            catch (Exception e) {
                Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
                finish();
                return;
            }

        getSupportActionBar().hide();
        setContentView(R.layout.activity_main);

        findViewById(R.id.div_bottom).setVisibility(View.VISIBLE);
        findViewById(R.id.div_extended).setVisibility(View.GONE);

        findViewById(R.id.more).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleMore();
            }
        });

        findViewById(R.id.camera_lib).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(v.getContext(), GalleryActivity.class));
            }
        });

        ((TextView)findViewById(R.id.version)).setText(getGStreamerVersion());

        initGS();
    }

    private void toggleMore() {

        if(findViewById(R.id.div_extended).getVisibility() == View.GONE)
            findViewById(R.id.div_extended).setVisibility(View.VISIBLE);

        else findViewById(R.id.div_extended).setVisibility(View.GONE);
    }

    @Override
    protected void onDestroy() {
        finalizeGS();
        super.onDestroy();
    }

    private void onGSReady() {

        isReady = true;

        runOnUiThread (new Runnable() {
            public void run() {
                final View v = findViewById(R.id.camera_button);
                v.setAlpha(1);
                v.setClickable(true);
            }
        });
    }

    private void onGSUnready() {

        isReady = false;

        runOnUiThread (new Runnable() {
            public void run() {
                final View v = findViewById(R.id.camera_button);
                v.setAlpha(0.5f);
                v.setClickable(false);
            }
        });
    }

    private native String getGStreamerVersion();
    private native void finalizeGS();
    private native void initGS();
    private static native boolean initNative();

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("phomera");
        canInit = initNative();
    }
}
