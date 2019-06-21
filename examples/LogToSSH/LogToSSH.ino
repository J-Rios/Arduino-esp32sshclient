/**************************************************************************************************/
/* Connect to SSH Server using password and log each 5s device counter value to ~/esp32.log file  */
/* in the Server.                                                                                 */
/**************************************************************************************************/

/* Libraries */

#include <WiFi.h>
#include <esp32sshclient.h>

/**************************************************************************************************/

// WiFi Parameters
#define WIFI_SSID "mynet1234"
#define WIFI_PASS "password1234"
#define MAX_CONN_FAIL 50
#define MAX_LENGTH_WIFI_SSID 31
#define MAX_LENGTH_WIFI_PASS 63

// SSH Connection Parameters
#define SSH_HOST "255.255.255.255"
#define SSH_PORT 22
#define SSH_USER "user"
#define SSH_PASS "pass"

/**************************************************************************************************/

/* Functions Prototypes */

void wifi_init_stat(void);
bool wifi_handle_connection(void);
void system_reboot(void);

/**************************************************************************************************/

/* Globals */

// Create and initialize SSH client object
ESP32SSHCLIENT ssh_client;

/**************************************************************************************************/

/* Main Function */

void setup(void)
{
    Serial.begin(115200);

    // Initialize WiFi station connection
    wifi_init_stat();
}

void loop()
{
    static const uint8_t MAX_CMD_LENGTH = 128;
    static uint32_t i = 0;
    char cmd[MAX_CMD_LENGTH];

    // Check if WiFi is connected
    if(!wifi_handle_connection())
    {
        // If SSH was connected, close and release all SSH resources
        if(ssh_client.is_connected())
            ssh_client.disconnect();

        // Wait 100ms and check again
        delay(100);
        return;
    }

    // Check if SSH client is not connected and try to connect
    if(!ssh_client.is_connected())
    {
        ssh_client.connect(SSH_HOST, SSH_PORT, SSH_USER, SSH_PASS);
        if(!ssh_client.is_connected())
        {
            // If not connected, wait 100ms and check again
            delay(100);
            return;
        }
    }

    // Send and execute the command through SSH
    snprintf(cmd, MAX_CMD_LENGTH, "echo \"Actual device counter: %d\" >> ~/esp32.log", i);
    ssh_client.send_cmd(cmd);
    i = i + 5;

    delay(5000);
}

/**************************************************************************************************/

/* Functions */

// Init WiFi interface
void wifi_init_stat(void)
{
    Serial.printf("Initializing TCP-IP adapter...\n");
    WiFi.mode(WIFI_OFF);
    if(WiFi.mode(WIFI_STA))
        Serial.printf("  [OK] TCP-IP adapter initialized.\n");
    else
    {
        Serial.printf("  [ERROR] TCP-IP adapter initialization fail.\n");
        system_reboot();
    }

    WiFi.disconnect(true, true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("Waiting Wifi connection to \"%s\"...\n", WIFI_SSID);
}

// Softreboot device
void system_reboot(void)
{
    Serial.printf("Rebooting system now.\n\n");
    fflush(stdout);
    esp_restart();
}

/**************************************************************************************************/

/* WiFi Change Event Handler */

bool wifi_handle_connection(void)
{
    static bool wifi_connected = false;

    // Device is not connected
    if(WiFi.status() != WL_CONNECTED)
    {
        // Was connected
        if(wifi_connected)
        {
            Serial.printf("\nWiFi disconnected.\n");
            wifi_connected = false;
        }

        return false;
    }
    // Device connected
    else
    {
        // Wasn't connected
        if(!wifi_connected)
        {
            Serial.printf("\nWiFi connected.\n");
            Serial.printf("IP address: ");
            Serial.println(WiFi.localIP());
            wifi_connected = true;
        }

        return true;
    }
}

