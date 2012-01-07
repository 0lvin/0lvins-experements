package airscanner.main;

import android.app.Activity;
import android.os.Bundle;
import android.telephony.NeighboringCellInfo;
import android.telephony.TelephonyManager;
import android.widget.TextView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;

import java.util.Iterator;
import java.util.List;

public class FrontActivity extends Activity {
	private TextView textWifi;
	private WifiReceiver receiverWifi;
	private WifiManager wifi;
	private TextView textGPS;
	private LocationManager location;
	
	class WifiReceiver extends BroadcastReceiver {
		public void onReceive(Context c, Intent intent) {
			List<ScanResult> wifiList;
			StringBuilder sb = new StringBuilder();
			sb.append("Wifi:On\n");
			wifiList = wifi.getScanResults();
	        for(int i = 0; i < wifiList.size(); i++){
	        	sb.append(new Integer(i+1).toString() + ".");
	        	sb.append((wifiList.get(i)).toString());
	        	sb.append("\n");
	        }
	        textWifi.setText(sb);
		}
	}
	
	private String wifiStatus() {
        String text = "";
        wifi = (WifiManager)this.getSystemService(Context.WIFI_SERVICE);
        if (!wifi.isWifiEnabled()) {
        	text += "Wifi:Off\n";
        } else {
        	text += "Wifi:On Scanning!\n";
        	receiverWifi = new WifiReceiver();
        	if (receiverWifi != null)
        		registerReceiver(receiverWifi, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
        	wifi.startScan();
        }
        return text;
	}
	
	protected void onPause() {
		if (receiverWifi != null)
			unregisterReceiver(receiverWifi);
		if (location != null && gpsListener != null) {
			location.removeGpsStatusListener(gpsListener);
			location.removeUpdates(gpsListener);
		}
		super.onPause();
	}

	protected void onResume() {
		if (receiverWifi != null)
			registerReceiver(receiverWifi, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
		if (location != null && gpsListener != null) {
			location.addGpsStatusListener(gpsListener);
			location.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 1000, gpsListener);	
		}
		super.onResume();
	}
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ScrollView sv = new ScrollView(this);
        {
        	LinearLayout mainTable = new LinearLayout(this);
        	mainTable.setOrientation(LinearLayout.VERTICAL);
        	{
        		textWifi = new TextView(this);
        		textWifi.setText(this.wifiStatus());
        		mainTable.addView(textWifi);
        	}
        	{
        		TextView textNet;
        		textNet = new TextView(this);
        		textNet.setText(this.netStastus());
        		mainTable.addView(textNet);
        	}   
        	{
        		TextView textPhone;
        		textPhone = new TextView(this);
        		textPhone.setText(this.phoneStastus());
        		mainTable.addView(textPhone);
        	}
        	{
        		textGPS = new TextView(this);
        		textGPS.setText(this.gpsStatus());
        		mainTable.addView(textGPS);
        	}
        	sv.addView(mainTable);
        }
        setContentView(sv);
    }
    
    private GPSListener gpsListener;
    class GPSListener implements android.location.GpsStatus.Listener, LocationListener {

		public void onGpsStatusChanged(int state) {
			if(state == GpsStatus.GPS_EVENT_SATELLITE_STATUS)
			{
				StringBuilder sb = new StringBuilder();
				sb.append("GPS:\n");
				GpsStatus status = location.getGpsStatus(null);
				if(status != null) {
					Iterable<GpsSatellite> satellites =  status.getSatellites();
					Iterator<GpsSatellite>sat = satellites.iterator();
					int i=0;
					while (sat.hasNext()) {
						GpsSatellite satellite = (sat).next();
						sb.append(i++);
						sb.append(": PRN#");
						sb.append(satellite.getPrn());
						if (satellite.usedInFix())
							sb.append(" Used in fix ");
						sb.append(" Signal/Noise:");
						sb.append(satellite.getSnr());
						sb.append(" Azimuth:");
						sb.append(satellite.getAzimuth());
						sb.append(" Elevation:");
						sb.append(satellite.getElevation());
						sb.append("\n");
					}
					textGPS.setText(sb.toString());
				}
			}
		}

		public void onLocationChanged(Location arg0) {
			// TODO Auto-generated method stub
			
		}

		public void onProviderDisabled(String arg0) {
			// TODO Auto-generated method stub
			
		}

		public void onProviderEnabled(String arg0) {
			// TODO Auto-generated method stub
			
		}

		public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
			// TODO Auto-generated method stub
			
		}
    	
    }
	private String gpsStatus() {
		location = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
		gpsListener = new GPSListener();
		location.addGpsStatusListener(gpsListener);
		location.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 1000, gpsListener);
		return "GPS scanning start";
	}

	private String phoneStastus() {
		TelephonyManager net = (TelephonyManager)this.getSystemService(Context.TELEPHONY_SERVICE);
		StringBuilder sb = new StringBuilder();
		sb.append("Phone: on\nOperator:");
		sb.append(net.getNetworkOperatorName());
		sb.append("\nLocation:");
		if(net.getCellLocation()!= null)
			sb.append(net.getCellLocation().toString());
		sb.append("\nType:");
		sb.append(net.getNetworkType());
		sb.append("\n");
		List<NeighboringCellInfo> cellList = net.getNeighboringCellInfo();
		for(int i = 0; i < cellList.size(); i++){
        	sb.append(new Integer(i+1).toString() + ".");
        	NeighboringCellInfo cell = cellList.get(i);
        	sb.append(" Signal Strength:");
        	sb.append( -113 + 2*cell.getRssi());
        	sb.append("dBm");
        	if (
        		cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_GPRS ||
        		cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_EDGE
        	) {
        		sb.append(" Cell:");
        		sb.append(cell.getCid());
        		sb.append(" LAC:");
        		sb.append(cell.getLac());
        	} else if ( 
        			cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_UMTS ||
        			cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_HSDPA ||
        			cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_HSUPA ||
        			cell.getNetworkType() == TelephonyManager.NETWORK_TYPE_HSPA
        	) {
        		sb.append(" Primary Scrambling Code:");
        		sb.append(cell.getPsc());
        	} else {
        		sb.append(" Network type is unknown!");
        	}
        	sb.append("\n");
        }
        return sb.toString();
	}

	private String netStastus() {
		ConnectivityManager net = (ConnectivityManager)this.getSystemService(Context.CONNECTIVITY_SERVICE);
		StringBuilder sb = new StringBuilder();
		sb.append("Network: on\n");
		NetworkInfo[] netList = net.getAllNetworkInfo();
        for(int i = 0; i < netList.length; i++){
        	sb.append(new Integer(i+1).toString() + ".");
        	sb.append((netList[i]).toString());
        	sb.append("\n");
        }
        return sb.toString();
	}
}