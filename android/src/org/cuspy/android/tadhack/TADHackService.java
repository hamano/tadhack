package org.cuspy.android.tadhack;

import java.util.Date;
import java.util.UUID;

import android.util.Log;
import android.app.Service;
import android.os.Handler;
import android.os.IBinder;
import android.os.Binder;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.widget.Toast;

import org.apache.http.client.HttpClient;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;

import org.json.JSONObject;
import org.json.JSONException;

import com.getpebble.android.kit.Constants;
import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.*;

public class TADHackService extends Service {
    private static final String tag = TADHackActivity.tag;
    private Context ctx;
    private IBinder binder = new TADHackBinder();
    private PebbleDataReceiver receiver;
    private String token;
    private String room;
    private String api;

    public TADHackService() {
        super();
        Log.v(tag, "TADHackService#TADHackService()");
    }

    public class TADHackBinder extends Binder{
        TADHackService getService(){
            return TADHackService.this;
        }
    }

    @Override
    public void onCreate() {
        Log.i(tag, "TADHackService#onCreate()");
        super.onCreate();
        ctx = getApplicationContext();
        Resources res = ctx.getResources();
        api = res.getString(R.string.api);
        token = res.getString(R.string.token);
        room = res.getString(R.string.room);
        join();
        room_message("start service");

        UUID appid = UUID.fromString("3aa69e1a-29e6-43c7-b7af-22a8a3064169");
        receiver = new PebbleKit.PebbleDataReceiver(appid) {
                @Override
                public void receiveData(Context context, int transactionId, PebbleDictionary data) {
                    Log.i(tag, "recv pebble: ");
                    Long value = data.getUnsignedIntegerAsLong(0);
                    if(value != null){
                        int action = value.intValue();
                        Log.i(tag, "action: " + action);
                        return;
                    }
                    value = data.getUnsignedIntegerAsLong(1);
                    if(value != null){
                        int state = value.intValue();
                        Log.i(tag, "state change: " + state);
                        return;
                    }
                }
            };
        PebbleKit.registerReceivedDataHandler(this, receiver);
    }

    @Override
    public void onDestroy() {
        Log.i(tag, "TADHackService#onDestroy()");
        super.onDestroy();
        unregisterReceiver(receiver);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(tag, "TADHackService#onBind()");
        return binder;
    }

    @Override
    public boolean onUnbind(Intent intent){
        Log.i(tag, "TADHackService#onUnbind()");
        return true;
    }

    private int post_data(String url, JSONObject data){
        HttpClient client = new DefaultHttpClient();
        HttpPost req = new HttpPost(url);
        int rc;
        try {
            StringEntity entity = new StringEntity(data.toString());
            req.setEntity(entity);
            HttpResponse res = client.execute(req);
            rc = res.getStatusLine().getStatusCode();
            return rc;
        } catch(Exception e){
            Log.e(tag, "e: " + e);
            return -1;
        }
    }

    private void room_message(String body){
        String url = api + "/rooms/" + room + "/send/m.room.message?access_token=" + token;
        JSONObject data = new JSONObject();
        try {
            data.put("msgtype", "m.text");
            data.put("body", body);
        } catch(Exception e){
            Log.e(tag, "error: " + e);
            return;
        }
        int rc = post_data(url, data);
        Log.i(tag, "rc: " + rc);
    }

    public void test() {
        Log.i(tag, "TADHackService#test()");
        Date date = new Date();
        room_message(date.toString());
    }


    public void join(){
        Log.i(tag, "TADHackService#join()");
        String url = api + "/join/" + room + "?access_token=" + token;
        JSONObject data = new JSONObject();
        int rc = post_data(url, data);
        Log.i(tag, "rc: " + rc);
    }

    public void leave(){
        Log.i(tag, "TADHackService#leave()");
        HttpClient client = new DefaultHttpClient();
        String url = api + "/rooms/" + room + "/leave?access_token=" + token;
        HttpPost req = new HttpPost(url);
        JSONObject msg = new JSONObject();
        int rc;
        try {
            StringEntity entity = new StringEntity(msg.toString());
            req.setEntity(entity);
            HttpResponse res = client.execute(req);
            rc = res.getStatusLine().getStatusCode();
            Log.i(tag, "rc: " + rc);
        } catch(Exception e){
            Log.e(tag, "e: " + e);
        }
    }
}
