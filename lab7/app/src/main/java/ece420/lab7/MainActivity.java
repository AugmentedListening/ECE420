package ece420.lab7;

import android.graphics.Color;
import android.graphics.Paint;
import android.support.v7.app.AppCompatActivity;
import android.support.v4.content.ContextCompat;
import android.support.v4.app.ActivityCompat;
import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.SurfaceHolder.Callback;
import java.io.IOException;
import android.graphics.Canvas;
import android.hardware.Camera.PreviewCallback;
import android.graphics.Rect;
import java.util.Vector;     //imports vector utility



import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.DataPointInterface;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.jjoe64.graphview.series.PointsGraphSeries;

public class MainActivity extends AppCompatActivity {

    private Camera camera;
    private SurfaceView surfaceView,transparentView;
    private SurfaceHolder surfaceHolder,holderTransparent;
    boolean previewing = false;
    boolean started = false;
    private TextView textHR;
    private Button buttonmain;
    // For Plotting The Data
    public LineGraphSeries<DataPoint> redlevelData;
    public LineGraphSeries<DataPoint> filteredData;
    public PointsGraphSeries<DataPoint> peakData;
    private int dataIdx = 0;
    // Camera Parameters and Window Size
    private int width = 640;
    private int height = 480;
    private final int winsize = 50;
    // Don't Modify Anything Above, Feel Free to Modify Anything below
    // Do whatever you want witht filter part, below is just an example
    public int len = 9; //N_TAPS
    public Vector<Double> rawbuffer = new Vector<Double>(len);
    public Vector<Double> filbuffer = new Vector<Double>(len);
    private double coefs[] = {-0.0156636074896, 0.0584972910598, 0.154487946952, 0.235710053896, 0.267502028052, 0.235710053896, 0.154487946952, 0.0584972910598, -0.0156636074896};// Declare Your variables for calculating heart rate here
    double alpha;
    int flag;
    // Time Variables
    private long someTime;
    // Mean Recording
    //private double prevMean;
    //private double newMean;
    private double runningMean;
    private int peakNum;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_DENIED){
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.CAMERA}, 1);}

        textHR = (TextView) findViewById(R.id.textViewHR);
        textHR.setText("--");
        buttonmain = (Button) findViewById(R.id.buttonMain);
        buttonmain.setText("Start");
        buttonmain.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!started) {
                    started = true;

                    buttonmain.setText("stop");
                    textHR.setText("--");

                    // If you want to Initialize anything when you hit button, put them here.
                    //prevMean = 0.0;
                    //newMean = 0.0;
                    runningMean = 1.0; // prevent divide by zero
                    peakNum = 0; // start at 0 for calculation sake
                    someTime = 0;
                    alpha = 0.95;
                    flag = 0;
                    // This part below is just to keep the app running
                    rawbuffer = new Vector<Double>(len);
                    filbuffer = new Vector<Double>(len);
                    for(int i=0;i<len;i++){
                        rawbuffer.add(0.0);
                        filbuffer.add(0.0); //don't do this or else filbuffer is too large
                    }

                }
                else{
                    started = false;
                    buttonmain.setText("start");
                }
            }
        });

        surfaceView = (SurfaceView)findViewById(R.id.surfaceCam);
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(new Callback() {
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width,
                                       int height) {
                // TODO Auto-generated method stub
            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {

                // TODO Auto-generated method stub
                if(!previewing) {
                    camera = Camera.open(1);
                    if (camera != null) {
                        try {
                            Camera.Parameters parameters = camera.getParameters();
                            parameters.setPreviewSize(width,height);
                            camera.setParameters(parameters);
                            camera.setDisplayOrientation(90);
                            camera.setPreviewDisplay(surfaceHolder);
                            camera.setPreviewCallback(new PreviewCallback() {
                                public void onPreviewFrame(byte[] data, Camera camera)
                                {

                                    // redlevel is the mean value from current frame
                                    double redlevel = calcRedMean(data);
                                    DataPoint newVal = new DataPoint(dataIdx, redlevel);
                                    redlevelData.appendData(newVal, true, 100);

                                    if(started){
                                        // We call your function here
                                        int result = calcHeartRate(redlevel);
                                        // Remember to modify the part below
                                        // We Plot the filtered data using the last element inside filbuffer vector
                                        newVal = new DataPoint(dataIdx, filbuffer.get(len-1));
                                        filteredData.appendData(newVal, true, 100);
                                        // We Plot the peaks marker using the second last element inside filbuffer vector
                                        if(result==1){
                                            DataPoint pVal = new DataPoint(dataIdx-1, filbuffer.get(len-2));
                                            peakData.appendData(pVal, true, 20);
                                        }
                                    }
                                    // We update our own plotting index, don't Modify this
                                    dataIdx ++;
                                }
                            });
                            camera.startPreview();
                            previewing = true;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                if (camera != null && previewing) {
                    camera.stopPreview();
                    camera.setPreviewCallback(null);
                    camera.release();
                    camera = null;
                    previewing = false;
                }
            }
        });
        surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        transparentView = (SurfaceView)findViewById(R.id.surfaceRect);
        holderTransparent = transparentView.getHolder();
        holderTransparent.addCallback(new Callback() {
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width,int height) {
            }
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Canvas canvas = holderTransparent.lockCanvas(null);
                Paint  paint = new Paint(Paint.ANTI_ALIAS_FLAG);
                paint.setStyle(Paint.Style.STROKE);
                paint.setColor(Color.CYAN);
                paint.setStrokeWidth(5);
                Rect rec=new Rect((canvas.getWidth()*(320-winsize)/640),(canvas.getHeight()*(240-winsize)/480),(canvas.getWidth()*(320+winsize)/640),(canvas.getHeight()*(240+winsize)/480));
                canvas.drawRect(rec,paint);
                holderTransparent.unlockCanvasAndPost(canvas);                                           // ´ò¿ªÉãÏñÍ·
            }
            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
            }
        });
        holderTransparent.setFormat(PixelFormat.TRANSLUCENT);
        transparentView.setZOrderMediaOverlay(true);

        GraphView Ograph = (GraphView) findViewById(R.id.viewO);
        redlevelData = new LineGraphSeries<>();
        Ograph.addSeries(redlevelData);
        Ograph.getViewport().setXAxisBoundsManual(true);
        Ograph.getViewport().setMinX(0);
        Ograph.getViewport().setMaxX(100);

        GraphView Fgraph = (GraphView) findViewById(R.id.viewF);
        filteredData = new LineGraphSeries<>();
        Fgraph.addSeries(filteredData);
        Fgraph.getViewport().setXAxisBoundsManual(true);
        Fgraph.getViewport().setMinX(0);
        Fgraph.getViewport().setMaxX(100);

        peakData = new PointsGraphSeries<>();
        Fgraph.addSeries(peakData);
        peakData.setColor(Color.GREEN);
        peakData.setCustomShape(new PointsGraphSeries.CustomShape() {
            @Override
            public void draw(Canvas canvas, Paint paint, float x, float y, DataPointInterface dataPoint) {
                paint.setStrokeWidth(8);
                canvas.drawLine(x-15, y-15, x+15, y+15, paint);
                canvas.drawLine(x+15, y-15, x-15, y+15, paint);
            }
        });
    }

    // Calculate Red Mean Value from YUV
    public double calcRedMean(byte[] data){
        final int frameSize = width * height;
        double redlevel = 0;

        for (int j = 0, yp = 0; j < height; j++) {
            int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
            for (int i = 0; i < width; i++, yp++) {
                if(j>(240-winsize) && j<= (240+winsize) && i>=(320-winsize) && i<=(320+winsize)){
                    int y = (0xff & ((int) data[yp])) - 16;
                    if (y < 0)
                        y = 0;
                    if ((i & 1) == 0) {
                        v = (0xff & data[uvp++]) - 128;
                        u = (0xff & data[uvp++]) - 128;
                    }

                    int y1192 = 1192 * y;
                    int r = (y1192 + 1634 * v);
                    //int g = (y1192 - 833 * v - 400 * u);
                    //int b = (y1192 + 2066 * u);

                    if (r < 0)                  r = 0;
                    else if (r > 262143)       r = 262143;
                    //if (g < 0)                  g = 0;
                    //else if (g > 262143)       g = 262143;
                    //if (b < 0)                  b = 0;
                    //else if (b > 262143)        b = 262143;

                    //rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
                    // Calculate the area with in height 240-50:240+50, width 320-50:320+50
                    redlevel += (double)((((r << 6) & 0xff0000)>>16) & 0x00ff);
                }
            }
        }
        return redlevel/((winsize*2+1)*(winsize*2+1));
    }

    // Implement this Function: Calculate Heart Rate
    public int calcHeartRate(double redmean) {
        int ispeak = 0;
        long PeakPeriod = 0; //be careful, as if you update running mean when no peak it returns incorect average
        int bpm = 0;
        // Feel Free to Modify anything below
        // The current code is just to keep this app running
        // Remove the oldest data and insert the newest data

        //Ex. |0|1|2|3|4| 0  is oldest, 4 is newest

        double sum = 0;
        /******** FILTER APPLICATION AND PEAK DETECTION ***************/
        if (rawbuffer.size() >= len) {
            rawbuffer.remove(0); //remove
        }
        rawbuffer.add(redmean);
        if(filbuffer.size() >= len)
        {
            filbuffer.remove(0);
        }
        for (int i = len - 1; i > -1; i--) {
            sum += rawbuffer.get(i) * coefs[-(i-len+1)];
        }
        filbuffer.add(sum); //oldest data is 0, so should be other end of coefs

        if ((filbuffer.get(filbuffer.size() - 2) - filbuffer.get(filbuffer.size() - 3)) > 0 && //check if previous value is smaller and new value is smaller
                (filbuffer.get(filbuffer.size() - 1) - filbuffer.get(filbuffer.size() - 2)) < 0) { //newdata is the last one, so check up to three back
            ispeak = 1;
            if (someTime == 0) {
                PeakPeriod = 0;
            } else {
                PeakPeriod = System.currentTimeMillis()/1000 - someTime; //find time between previous peak and current one
            }
            someTime = System.currentTimeMillis() / 1000; //update sometime
            flag = 1;
        }

        /********** RUNNING MEAN CALCULATION ************************/
        //System.out.println(PeakPeriod);

        if (redmean > 120) {
            if(flag == 1) {
                runningMean = alpha * runningMean + (1 - alpha) * PeakPeriod; //choose 0.8 as alpha
                flag = 0;
            }
            bpm = (int) (60 / runningMean);
            //peakNum += 1; // increment peak number
            textHR.setText(Integer.toString(bpm));
            // Return 0 for no peak found; Return 1 for peak found
        }
        return ispeak;
    }
}
