package net.osomi.phomera
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.view.View
import kotlinx.android.synthetic.main.activity_fullscreen.*
import androidx.core.app.ComponentActivity
import androidx.core.app.ComponentActivity.ExtraData
import androidx.core.content.ContextCompat.getSystemService
import android.icu.lang.UCharacter.GraphemeClusterBreak.T
import android.view.Window
import android.view.WindowManager

class FullscreenActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {

        super.onCreate(savedInstanceState)

        supportActionBar?.hide()
        setContentView(R.layout.activity_fullscreen)

        div_bottom.visibility = View.VISIBLE
        div_extended.visibility = View.GONE;

        more.setOnClickListener { toggleMore() }

        version.text = version.text.toString() + getGStreamerVersion();
    }

    private fun toggleMore() {

        if(div_extended.visibility == View.GONE)
            div_extended.visibility = View.VISIBLE;

        else div_extended.visibility = View.GONE;
    }

    external fun getGStreamerVersion(): String

    companion object {
        init {
            // System.loadLibrary("gstreamer_android")
            System.loadLibrary("phomera")
        }
    }
}
