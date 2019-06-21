/**************************************************************************************************/
/* Connect to SSH Server using password and execute each command send by serial.                  */
/*                                                                                                */
/* A command is all serial bytes that end with an \n EOL, so make sure to set                     */
/* "New line" in serial terminal.                                                                 */
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
    static const uint16_t MAX_CMD_LENGTH = 256;
    static char command[MAX_CMD_LENGTH] = { 0 };
    static uint8_t i = 0;
    static bool command_received = false;
    char rx_char;

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

    // Read from Serial while data available and not a command was received
    while((Serial.available()) && (!command_received))
    {
        rx_char = (char)Serial.read();
        //Serial.print(rx_char);

        if(i < MAX_CMD_LENGTH-1)
            command[i] = rx_char;
        i = i + 1;

        if(i == MAX_CMD_LENGTH-1)
        {
            command[MAX_CMD_LENGTH-1] = '\0';
            command_received = true;
            i = 0;
        }
        else
        {
            if(command[i-1] == '\n')
            {
                command[i-1] = '\0';
                command_received = true;
                i = 0;
            }
        }
    }

    // Check for received Serial commands
    if(command_received)
    {
        // Send and execute the command through SSH
        ssh_client.send_cmd(command);

        // Clear handle comand data
        memset(command, '\0', MAX_CMD_LENGTH);
        command_received = false;
    }    

    // Wait 250ms for next iteration
    delay(250);
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

