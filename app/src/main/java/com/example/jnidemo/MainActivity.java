package com.example.jnidemo; // Defines the Java package used by the app and JNI symbol names.

import android.graphics.Color; // Provides readable color constants for status messages.
import android.os.Bundle; // Carries Activity state information during creation.
import android.widget.TextView; // Displays the security status and JNI demo output.

import androidx.appcompat.app.AppCompatActivity; // Gives the Activity AppCompat behavior.

public class MainActivity extends AppCompatActivity { // Main screen for the JNI anti-debug lab app.

    static { // Runs once when the class is first loaded by Android.
        System.loadLibrary("native-lib"); // Loads the C++ shared library named libnative-lib.so.
    }

    private TextView tvStatus; // Shows whether the environment looks safe or suspicious.
    private TextView tvHello; // Shows the JNI greeting when the environment is safe.
    private TextView tvFact; // Shows the native factorial result when the environment is safe.

    public native boolean isDebugDetected(); // Asks native C++ code to run defensive checks.

    public native String helloFromJNI(); // Gets a simple educational string from native C++ code.

    public native int factorial(int n); // Calculates a factorial in native C++ code.

    @Override
    protected void onCreate(Bundle savedInstanceState) { // Called when Android creates this Activity.
        super.onCreate(savedInstanceState); // Lets AppCompat perform its normal setup work.
        setContentView(R.layout.activity_main); // Loads the XML layout containing the three TextViews.

        tvStatus = findViewById(R.id.tvStatus); // Finds the status TextView from the layout.
        tvHello = findViewById(R.id.tvHello); // Finds the greeting TextView from the layout.
        tvFact = findViewById(R.id.tvFact); // Finds the factorial TextView from the layout.

        boolean suspicious = isDebugDetected(); // Runs the native anti-debug detection before demo output.

        if (suspicious) { // Uses the blocked UI path when native code reports a suspicious state.
            tvStatus.setTextColor(Color.RED); // Colors the security status red for a warning state.
            tvStatus.setText("Etat securite : environnement suspect detecte"); // Shows the requested warning text.
            tvHello.setText("Fonction native sensible desactivee"); // Blocks the normal JNI greeting output.
            tvFact.setText("Calcul natif bloque"); // Blocks the normal JNI calculation output.
        } else { // Uses the normal demo UI path when the native checks do not detect suspicion.
            tvStatus.setTextColor(Color.rgb(0, 128, 0)); // Colors the security status green for an OK state.
            tvStatus.setText("Etat securite : OK"); // Shows the requested safe-state text.
            tvHello.setText(helloFromJNI()); // Calls the native greeting only after the security check passes.
            tvFact.setText("Factoriel de 10 = " + factorial(10)); // Calls the native factorial only when safe.
        }
    }
}
