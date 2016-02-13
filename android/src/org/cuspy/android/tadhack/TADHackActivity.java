package org.cuspy.android.tadhack;

import android.app.Activity;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.content.res.Resources;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Menu;
import android.view.MenuItem;

public class TADHackActivity extends Activity
{
    public static final String tag = "tadhack";
    private TADHackService service;
    private String token;
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.i(tag, "TADHackActivity#onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        OnClickListener startButton = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent(getApplication(),
                                               TADHackService.class);
                    bindService(intent, connection, Context.BIND_AUTO_CREATE);
                }
            };
        findViewById(R.id.start).setOnClickListener(startButton);
        OnClickListener stopButton = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if(service != null){
                        Log.i(tag, "unbind");
                        unbindService(connection);
                        service = null;
                    }
                }
            };
        findViewById(R.id.stop).setOnClickListener(stopButton);
        OnClickListener joinButton = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if(service != null){
                        service.join();
                    }
                }
            };
        findViewById(R.id.join).setOnClickListener(joinButton);
        OnClickListener leaveButton = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if(service != null){
                        service.leave();
                    }
                }
            };
        findViewById(R.id.leave).setOnClickListener(leaveButton);
    }

    /** Defines callbacks for service binding, passed to bindService() */
    private ServiceConnection connection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName className,
                                           IBinder binder) {
                Log.i(tag, "ServiceConnection#onServiceConnected()");
                service = ((TADHackService.TADHackBinder)binder).getService();
            }

            @Override
            public void onServiceDisconnected(ComponentName arg0) {
                Log.i(tag, "ServiceConnection#onServiceDisconnected()");
                service = null;
            }
        };

    @Override
    public boolean onCreateOptionsMenu(Menu menu){
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.test_walk:
            if(service != null){
                service.walk();
            }
            break;
        case R.id.test_run:
            if(service != null){
                service.run();
            }
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onDestroy() {
        Log.i(tag, "TADHackActivity#onDestroy()");
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        Log.i(tag, "TADHackActivity#onResume()");
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.i(tag, "TADHackActivity#onPause()");
        super.onPause();
    }
}
