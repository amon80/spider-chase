package spiderwars.robotics.unisa.it.robowarspad;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.zerokol.views.JoystickView;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;

/**
 * @author Simone Romano 0522501294
 */
public class MainActivity extends AppCompatActivity implements JoystickView.OnJoystickMoveListener {
    private String param = "M0000000";
    private String lastParam = "M0000000";
    static String MOTOR = "VAL";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        JoystickView jv = (JoystickView) findViewById(R.id.joystickView);
        jv.setOnJoystickMoveListener(this, JoystickView.DEFAULT_LOOP_INTERVAL);
    }

    @Override
    public void onValueChanged(int angle, int power, int direction) {
        Log.i(MOTOR, angle + " " + power + " " + direction);

        if(inRange(angle,-10,10) && inRange(power,-10,10) && direction==0){   //STOP
            param = "M0127127";
        }
        if (inRange(angle,80,100) && inRange(power,80,110) && direction==1) {   //RIGHT
            param = "M0128255";
        }
        if (inRange(angle,10,80) && inRange(power,80,100) && direction== 2){ //RIGHT AHEAD
            param = "M0128218";
        }
        if (inRange(angle,-100,-70) && inRange(power,90,110) && direction==5) {   //LEFT
            param = "M0255128";
        }
        if (inRange(angle,-70,-10) && inRange(power,90,110) && direction==4) {   //LEFT AHEAD
            param = "M0218128";
        }
        if (inRange(angle,-10,10) && inRange(power,90,110) && direction==3) {   //AHEAD
            param = "M0128128";
        }
        if ((inRange(angle,-180,-170) || inRange(angle,170,180)) && inRange(power,50,110) && direction==7) {   //BEHIND
            param = "M0000000";
        }
        if (inRange(angle,-170,-100) && inRange(power,50,110) && direction==6) {   //BEHIND LEFT
            param = "M0000090";
        }
        if (inRange(angle,100,170) && inRange(power,50,110) && direction==8) {   //BEHIND RIGHT
            param = "M0090000";
        }
        if (!param.equals(lastParam))
            new HttpReq().execute(param);
        lastParam = param;
        Log.i(MOTOR, lastParam);
    }

    public boolean inRange(int val, int lower, int upper){
        if (val<=upper && val>=lower)
            return true;
        return false;
    }
}
