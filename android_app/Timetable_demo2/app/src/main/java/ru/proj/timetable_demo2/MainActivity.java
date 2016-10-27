package ru.proj.timetable_demo2;

import android.app.ActionBar;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.DisplayMetrics;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import ru.proj.timetable_demo2.SwipeDetector;
import ru.proj.timetable_demo2.Constants;

import static android.view.View.*;

public class MainActivity extends AppCompatActivity {
    private GestureDetector gestureDetector;
    private String data = "", date = "";
    private int width = 0;
    final String FILENAME = "storage";
    //private final String serverAddr = "http://37.194.33.140/timetable.php?v=2";
    //private final String serverAddr = "http://192.168.0.36/timetable.php?v=2";
    private String serverAddr = new Constants().serverAddr;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        /*получаем ширину экрана*/
        /*DisplayMetrics metrics;
        metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        width = metrics.widthPixels;

        LinearLayout ll = (LinearLayout) findViewById(R.id.mainList);
        ll.getLayoutParams().width = width;*/

        /*gestureDetector = initGestureDetector();

        View view = findViewById(R.id.scroll_area);
        view.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View v, MotionEvent event) {
                return gestureDetector.onTouchEvent(event);
            }
        });

        view.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
            }
        });*/
        new LoadTask().execute();
    }
    private class LoadTask extends AsyncTask<Void, Void, String> {
        @Override
        protected String doInBackground(Void... params) {
            if(!LoadFromHttp())
                if(!LoadFromStorage())
                    return "";
            return data;
        }
        @Override
        protected void onPostExecute(String strJson) {
            super.onPostExecute(strJson);
            Calendar calendar = Calendar.getInstance();
            if(calendar.get(Calendar.DAY_OF_WEEK) == Calendar.SUNDAY)
                calendar.add(Calendar.DAY_OF_YEAR, 1);
            Date d = calendar.getTime();
            SimpleDateFormat sdf = new SimpleDateFormat("dd.MM");
            date = sdf.format(d);
            generateByDate();
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
            SaveToStorage();
            return true;
        }
        private boolean LoadFromStorage()
        {
            try {
                // открываем поток для чтения
                BufferedReader br = new BufferedReader(new InputStreamReader(
                        openFileInput(FILENAME)));
                String str = "";
                // читаем содержимое
                while ((str = br.readLine()) != null) {
                    data += str;
                }
                // закрываем поток
                br.close();
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
            return true;
        }
        private void SaveToStorage(){
            try {
                // отрываем поток для записи
                BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(
                        openFileOutput(FILENAME, MODE_PRIVATE)));
                // пишем данные
                bw.write(data);
                // закрываем поток
                bw.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    private void generateByDate(){
        //TextView today = (TextView) findViewById(R.id.today);
        JSONArray dataJsonArray = null;
        try {
            dataJsonArray = new JSONArray(data);
            JSONObject day = null;
            int i;
            String tmpDate = "";
            LinearLayout ll = (LinearLayout) findViewById(R.id.mainList);
            ll.removeAllViews();
            ScrollView sv = (ScrollView) findViewById(R.id.scroll_area);
            for (i = 0; i < dataJsonArray.length(); i++){
                day = dataJsonArray.getJSONObject(i);
                /*if(day.getString("date").equals(date))
                    break;*/
                tmpDate = day.getString("date");
                String dayOfWeek = "";
                switch(day.getInt("day")){
                    case 1: dayOfWeek = "пн "; break;
                    case 2: dayOfWeek = "вт "; break;
                    case 3: dayOfWeek = "ср "; break;
                    case 4: dayOfWeek = "чт "; break;
                    case 5: dayOfWeek = "пт "; break;
                    case 6: dayOfWeek = "сб "; break;
                }
                //today.setText(dayOfWeek + day.getString("date"));
                TextView btn = (TextView) LayoutInflater.from(this).inflate(R.layout.daybtn, null);
                btn.setText(dayOfWeek + day.getString("date"));
                ll.addView(btn);
                do{
                    View view = LayoutInflater.from(this).inflate(R.layout.msg, null);
                    TextView time = (TextView) view.findViewById(R.id.time);
                    time.setText(day.getString("time"));
                    TextView room = (TextView) view.findViewById(R.id.room);
                    room.setText(day.getString("room"));
                    TextView obj = (TextView) view.findViewById(R.id.obj);
                    obj.setText(day.getString("obj"));
                    TextView type = (TextView) view.findViewById(R.id.type);
                    type.setText(day.getString("type"));
                    TextView teacher = (TextView) view.findViewById(R.id.teacher);
                    teacher.setText(day.getString("teacher"));
                    ll.addView(view);
                    /*if(day.getString("date").equals("08.09"))
                        viewScroll = view;*/
                    day = dataJsonArray.getJSONObject(++i);
                }while(day.getString("date").equals(tmpDate));
                i--;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void goNext(){
        try {
            JSONArray dataJsonArray = new JSONArray(data);
            JSONObject day = null;
            for (int i = 0; i < dataJsonArray.length(); i++){
                day = dataJsonArray.getJSONObject(i);
                if(day.getString("date").equals(date)){
                    while(day.getString("date").equals(date) && i < dataJsonArray.length())
                        day = dataJsonArray.getJSONObject(++i);
                    date = day.getString("date");
                    break;
                }
            }
            generateByDate();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void goLast(){
        try {
            JSONArray dataJsonArray = new JSONArray(data);
            JSONObject day = null;
            for (int i = dataJsonArray.length()-1; i >= 0; i--){
                day = dataJsonArray.getJSONObject(i);
                if(day.getString("date").equals(date)){
                    while(day.getString("date").equals(date) && i >= 0)
                        day = dataJsonArray.getJSONObject(--i);
                    date = day.getString("date");
                    break;
                }
            }
            generateByDate();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }
    private GestureDetector initGestureDetector() {
        return new GestureDetector(new GestureDetector.SimpleOnGestureListener() {

            private SwipeDetector detector = new SwipeDetector();

            public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                                   float velocityY) {
                try {
                    if (detector.isSwipeDown(e1, e2, velocityY)) {
                        return false;
                    } else if (detector.isSwipeUp(e1, e2, velocityY)) {
                        return false;
                    }else if (detector.isSwipeLeft(e1, e2, velocityX)) {
                        goNext();
                    } else if (detector.isSwipeRight(e1, e2, velocityX)) {
                        goLast();
                    }
                } catch (Exception e) {} //for now, ignore
                return false;
            }

            private void showToast(String phrase){
                Toast.makeText(getApplicationContext(), phrase, Toast.LENGTH_SHORT).show();
            }
        });
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
