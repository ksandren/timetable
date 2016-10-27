package ru.proj.timetable_demo2;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class selectGroup extends AppCompatActivity {
    private String serverAddr = new Constants().serverAddr;
    private String data;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.select_group);
        new LoadTask().execute();
    }
    private class LoadTask extends AsyncTask<Void, Void, String> {
        @Override
        protected String doInBackground(Void... params) {
            if(!LoadFromHttp())
                return "";
            return data;
        }
        @Override
        protected void onPostExecute(String strJson) {
        }
        private boolean LoadFromHttp(){
            BufferedReader reader=null;
            try {
                URL url=new URL(serverAddr);
                HttpURLConnection c=(HttpURLConnection)url.openConnection();
                c.setRequestMethod("GET");
                c.setReadTimeout(10000);
                c.connect();
                reader = new BufferedReader(new InputStreamReader(c.getInputStream()));
                StringBuilder buf=new StringBuilder();
                String line=null;
                while ((line=reader.readLine()) != null) {
                    buf.append(line + "\n");
                }
                data = buf.toString();
                reader.close();
                c.disconnect();
            }
            catch (Exception e) {
                e.printStackTrace();
                return false;
            }
            return true;
        }
    }
}
