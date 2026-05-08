#include <jni.h> // Provides JNI types such as JNIEnv, jobject, jboolean, and jstring.
#include <android/log.h> // Provides Android Logcat functions for native logging.
#include <sys/ptrace.h> // Provides ptrace constants and calls used for tracing detection.
#include <cerrno> // Provides errno so ptrace failures can be interpreted safely.
#include <cstdio> // Provides FILE, fopen, fgets, and fclose for /proc/self/maps inspection.
#include <cstring> // Provides strstr for simple suspicious-name matching.
#include <string> // Provides std::string for the JNI greeting.

#define LOG_TAG "ANTI_DEBUG" // Sets a stable Logcat tag for this defensive native code.
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__) // Logs normal informational states.
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__) // Logs suspicious or warning states.
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__) // Logs unexpected error states.

static bool isBeingTraced() { // Checks whether this process appears to be traced with ptrace.
    errno = 0; // Clears errno before ptrace so the result can be interpreted cleanly.
    long result = ptrace(PTRACE_TRACEME, 0, nullptr, nullptr); // Requests tracing permission as a defensive signal.

    if (result == -1) { // A failed ptrace request can indicate that another tracer is already attached.
        if (errno == EPERM) { // EPERM is the suspicious case for an already-traced process.
            LOGW("ptrace check reported EPERM, process may already be traced"); // Records the suspicious ptrace state.
            return true; // Reports that a debugger or tracer may be present.
        }

        LOGE("ptrace check failed with errno=%d", errno); // Records unexpected ptrace failures for lab visibility.
        return false; // Avoids crashing or blocking on unclear errors in this educational demo.
    }

    ptrace(PTRACE_DETACH, 0, nullptr, nullptr); // Attempts a best-effort cleanup after the educational ptrace check.
    LOGI("ptrace check did not detect an external tracer"); // Records a normal ptrace result.
    return false; // Reports that this check did not detect suspicious tracing.
}

static bool containsSuspiciousLibraryNames() { // Scans loaded memory mappings for known instrumentation names.
    const char* suspiciousNames[] = { // Lists suspicious lowercase substrings for defensive detection.
            "frida", // Matches Frida-related libraries or mappings.
            "libfrida", // Matches explicit libfrida names.
            "xposed", // Matches Xposed-related mappings.
            "gdbserver", // Matches gdbserver debugger helper mappings.
            "libgdb", // Matches libgdb-related mappings.
            "magisk" // Matches Magisk-related mappings.
    };

    FILE* mapsFile = fopen("/proc/self/maps", "r"); // Opens this process mapping list for read-only inspection.
    if (mapsFile == nullptr) { // Handles the rare case where maps cannot be opened.
        LOGE("unable to open /proc/self/maps for inspection"); // Logs the failure without crashing the app.
        return false; // Chooses a non-suspicious result when the lab check cannot read maps.
    }

    char line[1024]; // Stores one line from /proc/self/maps at a time.
    while (fgets(line, sizeof(line), mapsFile) != nullptr) { // Reads each mapping line safely with a fixed buffer.
        for (const char* suspiciousName : suspiciousNames) { // Checks each configured suspicious substring.
            if (strstr(line, suspiciousName) != nullptr) { // Detects a suspicious library or mapping name.
                LOGW("suspicious mapping detected: %s", suspiciousName); // Logs which suspicious keyword matched.
                fclose(mapsFile); // Closes the maps file before returning from the detection path.
                return true; // Reports that a suspicious library or mapping was found.
            }
        }
    }

    fclose(mapsFile); // Closes the maps file after completing the scan.
    LOGI("/proc/self/maps check did not find suspicious library names"); // Records a normal maps-inspection result.
    return false; // Reports that no suspicious mapping names were detected.
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_jnidemo_MainActivity_isDebugDetected(JNIEnv* env, jobject thiz) { // Exposes the defensive check to Java.
    (void) env; // Marks the unused JNI environment parameter intentionally unused.
    (void) thiz; // Marks the unused Java object parameter intentionally unused.

    bool traced = isBeingTraced(); // Runs the ptrace-based defensive check.
    bool suspiciousLibraries = containsSuspiciousLibraryNames(); // Runs the /proc/self/maps suspicious-name check.

    if (traced || suspiciousLibraries) { // Combines both defensive signals into one Java result.
        LOGW("debug or instrumentation suspicion detected"); // Logs the blocked state for Logcat review.
        return JNI_TRUE; // Tells Java to disable the normal JNI demo outputs.
    }

    LOGI("no suspicious debug state detected"); // Logs the safe state for Logcat review.
    return JNI_FALSE; // Tells Java the environment appears acceptable for the demo outputs.
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_jnidemo_MainActivity_helloFromJNI(JNIEnv* env, jobject thiz) { // Exposes a simple native greeting to Java.
    (void) thiz; // Marks the unused Java object parameter intentionally unused.
    std::string hello = "Hello from C++ via JNI !"; // Creates the requested educational native message.
    return env->NewStringUTF(hello.c_str()); // Converts the C++ string into a Java String.
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_jnidemo_MainActivity_factorial(JNIEnv* env, jobject thiz, jint n) { // Exposes native factorial calculation to Java.
    (void) env; // Marks the unused JNI environment parameter intentionally unused.
    (void) thiz; // Marks the unused Java object parameter intentionally unused.

    if (n < 0) { // Handles invalid negative input defensively.
        LOGW("factorial received a negative value"); // Logs the invalid lab input.
        return 0; // Returns a simple safe value instead of crashing or throwing.
    }

    jint result = 1; // Starts the factorial accumulator at the multiplicative identity.
    for (jint i = 2; i <= n; ++i) { // Multiplies each integer from 2 through n.
        result *= i; // Updates the factorial result.
    }

    LOGI("factorial calculated for n=%d", n); // Logs normal native calculation activity.
    return result; // Returns the calculated factorial to Java.
}
