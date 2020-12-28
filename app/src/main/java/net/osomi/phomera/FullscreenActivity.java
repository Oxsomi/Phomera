package net.osomi.phomera;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import android.widget.TextView;

import org.freedesktop.gstreamer.GStreamer;

public class FullscreenActivity extends AppCompatActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        try {
            GStreamer.init(this);
        }
        catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        getSupportActionBar().hide();
        setContentView(R.layout.activity_fullscreen);

        findViewById(R.id.div_bottom).setVisibility(View.VISIBLE);
        findViewById(R.id.div_extended).setVisibility(View.GONE);

        findViewById(R.id.more).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleMore();
            }
        });

        ((TextView)findViewById(R.id.version)).setText(getGStreamerVersion());
    }

    private void toggleMore() {

        if(findViewById(R.id.div_extended).getVisibility() == View.GONE)
            findViewById(R.id.div_extended).setVisibility(View.VISIBLE);

        else findViewById(R.id.div_extended).setVisibility(View.GONE);
    }

    public native String getGStreamerVersion();

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("phomera");
    }
}
