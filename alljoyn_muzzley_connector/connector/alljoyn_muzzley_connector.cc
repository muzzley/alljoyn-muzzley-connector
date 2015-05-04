/*****************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 * Copyright (c) 2014, Muzzley
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

//Alljoyn Lighting SDK

#include <ControllerClient.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>
#include <ControllerServiceManager.h>
#include <qcc/StringUtil.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Debug.h>

//GUPnP
#include <libgupnp/gupnp.h>
#include <libgssdp/gssdp.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <gmodule.h>

//Muzzley
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <muzzley/muzzley.h>
#include <muzzley/parsers/http.h>
#include <muzzley/stream/SSLSocketStreams.h>

//Semaphore
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//Unordered_map
#include <unordered_map>
#include <tuple>

//Math
#include <cmath>
#include <math.h>


//Timer
#include <iostream>
#include <cstdio>
#include <ctime>
#include <chrono>

//Thread
#include <iostream>
#include <thread>

//Alljoyn Services
#include <CommonSampleUtil.h>
#include <AnnounceHandlerImpl.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <alljoyn/notification/Notification.h>
#include <alljoyn/notification/NotificationReceiver.h>
#include <alljoyn/notification/NotificationService.h>
#include <alljoyn/config/ConfigClient.h>
#include <alljoyn/services_common/LogModulesNames.h>
#include <alljoyn/services_common/GuidUtil.h>
#include <alljoyn/PasswordManager.h>
#include <alljoyn/controlpanel/ControlPanelService.h>
#include <alljoyn/controlpanel/ControlPanelController.h>
#include <ControlPanelListenerImpl.h>
#include <ControllerNotificationReceiver.h>

#include <alljoyn/controlpanel/ActionWithDialog.h>
#include <alljoyn/controlpanel/Container.h>
#include <alljoyn/controlpanel/Property.h>
#include <alljoyn/controlpanel/Dialog.h>
#include <alljoyn/controlpanel/Action.h>

#define MUZZLEY_DEFAULT_PLUGS_APP_TOKEN "b640b74e19f831a3"
#define MUZZLEY_DEFAULT_PLUGS_PROFILEID "54aaee6cec3302c4272076a2"
#define MUZZLEY_DEFAULT_LIGHTING_APP_TOKEN "5f09fd4b3b6f8821"
#define MUZZLEY_DEFAULT_LIGHTING_PROFILEID "5486e2e3ec3302c42720738b"
#define MUZZLEY_DEFAULT_CORE_ENDPOINTHOST "geoplatform.muzzley.com"
#define MUZZLEY_DEFAULT_API_ENDPOINTHOST "channels.muzzley.com"
#define MUZZLEY_DEFAULT_MANAGER_ENDPOINTHOST "global-manager.muzzley.com"
#define MUZZLEY_DEFAULT_API_PORT 80
#define MUZZLEY_DEFAULT_MANAGER_PORT 80

#define MUZZLEY_DEFAULT_MANAGER_REGISTER_URL "/deviceapp/register"
#define MUZZLEY_DEFAULT_MANAGER_COMPONENTS_URL "/deviceapp/components"
#define MUZZLEY_SEMAPHORE_FILENAME "muzzley_semaphore.txt"
#define MUZZLEY_LIGHTING_DEVICEKEY_FILENAME "lighting_key.txt"
#define MUZZLEY_PLUGS_DEVICEKEY_FILENAME "plugs_key.txt"
#define MUZZLEY_LIGHTING_XML_FILENAME "muzzley_lighting.xml"
#define MUZZLEY_PLUGS_XML_FILENAME "muzzley_plugs.xml"
#define MUZZLEY_WORKSPACE "iot"
#define MUZZLEY_UNKNOWN_NAME "---"
#define MUZZLEY_LOOPASSYNCHRONOUS true
#define MUZZLEY_BRIDGE_INFO false
#define MUZZLEY_READ_REQUEST_TIMEOUT 30

#define COLOR_WHITE "white"
#define COLOR_SILVER "silver"
#define COLOR_GRAY "gray"
#define COLOR_BLACK "black"
#define COLOR_RED "red"
#define COLOR_MAROON "maroon"
#define COLOR_YELLOW "yellow"
#define COLOR_OLIVE "olive"
#define COLOR_LIME "lime"
#define COLOR_GREEN "green"
#define COLOR_AQUA "aqua"
#define COLOR_TEAL "teal"
#define COLOR_BLUE "blue"
#define COLOR_NAVY "navy"
#define COLOR_FUCHSIA "fuschia"
#define COLOR_PURPLE "purple"

#define COLOR_MIN 0
#define COLOR_MAX_UINT32 4294967296
#define COLOR_MAX_PERCENT 100
#define COLOR_MAX_360DEG 360        
#define COLOR_TEMPERATURE_MIN_DEC 1000
#define COLOR_TEMPERATURE_MAX_DEC 20000
#define COLOR_TEMPERATURE_DEFAULT_DEC 3600
#define COLOR_RGB_MAX 255
#define COLOR_DOUBLE_MAX 1

#define GUPNP_MAX_AGE 1800
#define GUPNP_MESSAGE_DELAY 120

#define DEVICE_PLUG "plug"
#define DEVICE_BULB "bulb"
#define DEVICE_BRIDGE "bridge"
#define PROPERTY_STATUS "status"
#define PROPERTY_COLOR_RGB "color"
#define PROPERTY_COLOR_HSV "color-hsv"
#define PROPERTY_COLOR_HSVT "color-hsvt"
#define PROPERTY_COLOR_NAME "color-name"
#define PROPERTY_BRIGHTNESS "brightness"
#define PROPERTY_VOLTAGE "voltage"
#define PROPERTY_CURRENT "current"
#define PROPERTY_FREQUENCY "freq"
#define PROPERTY_POWER "power"
#define PROPERTY_ENERGY "energy"

#define MUZZLEY_DEFAULT_LIGHTING_FRIENDLYNAME "My Alljoyn Bulbs"
#define MUZZLEY_DEFAULT_LIGHTING_UDN "muzzley-lighting-udn"
#define MUZZLEY_DEFAULT_LIGHTING_SERIALNUMBER "muzzley-lighting-serialnumber-lisbon-office-1"
#define MUZZLEY_DEFAULT_PLUGS_FRIENDLYNAME "My Alljoyn Plugs"
#define MUZZLEY_DEFAULT_PLUGS_UDN "muzzley-plugs-udn"
#define MUZZLEY_DEFAULT_PLUGS_SERIALNUMBER "muzzley-plugs-serialnumber-lisbon-office-1"
#define MUZZLEY_DEFAULT_MANUFACTURER "Muzzley"
#define MUZZLEY_DEFAULT_MANUFACTURER_URL "www.muzzley.com"
#define MUZZLEY_DEFAULT_MODELDESCRIPTION "Muzzley Alljoyn Connector with UPnP Support"
#define MUZZLEY_DEFAULT_MODELNAME "Muzzley Alljoyn Connector"
#define MUZZLEY_DEFAULT_MODELNUMBER "0.1"

#define MUZZLEY_DEFAULT_NETWORK_INTERFACE "br-lan"
#define MUZZLEY_DEFAULT_NETWORK_LIGHTING_PORT 50000
#define MUZZLEY_DEFAULT_NETWORK_PLUGS_PORT 51000
#define MUZZLEY_DEFAULT_STATUS_INTERVAL 60

//Mac Address
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>


using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

using namespace qcc;
using namespace lsf;
using namespace ajn;
using namespace services;

string muzzley_color_mode="";
string muzzley_core_endpointhost="";
string muzzley_api_endpointhost="";
string muzzley_manager_endpointhost="";
string muzzley_manager_register_url="";
string muzzley_manager_components_url="";
string muzzley_lighting_profileid="";
string muzzley_lighting_apptoken="";
string muzzley_lighting_upnp_friendlyname="";
string muzzley_lighting_upnp_udn="";
string muzzley_lighting_upnp_serialnumber="";
string muzzley_lighting_upnp_urn="";
string muzzley_lighting_upnp_host="";
string muzzley_lighting_upnp_xml_filepath="";
string muzzley_lighting_upnp_interface="";
string muzzley_lighting_sessionid="";
string muzzley_lighting_macAddress="";
string muzzley_lighting_deviceKey="";
int muzzley_lighting_upnp_port=0;

string muzzley_plugs_profileid="";
string muzzley_plugs_apptoken="";
string muzzley_plugs_upnp_friendlyname="";
string muzzley_plugs_upnp_udn="";
string muzzley_plugs_upnp_serialnumber="";
string muzzley_plugs_upnp_urn="";
string muzzley_plugs_upnp_host="";
string muzzley_plugs_upnp_xml_filepath="";
string muzzley_plugs_upnp_interface="";
string muzzley_plugs_sessionid="";
string muzzley_plugs_macAddress="";
string muzzley_plugs_deviceKey="";
int muzzley_plugs_upnp_port=0;

string muzzley_network_interface="";
string muzzley_manufacturer="";
string muzzley_manufacturer_url="";
string muzzley_modeldescription="";
string muzzley_modelname="";
string muzzley_modelnumber="";
int muzzley_api_port=0;
int muzzley_manager_port=0;
bool muzzley_OnBehalfOf=true;

LSFString muzzley_controllerservice_id;
LSFString muzzley_controllerservice_name;
bool muzzley_controllerservice_connected=false;
bool muzzley_lighting_registered=false;
bool muzzley_plugs_registered=false;
LSFStringList lampList;

AnnounceHandlerImpl* announceHandler=0;
ControlPanelService* controlPanelService=0;
ControlPanelController* controlPanelController=0;
ControlPanelListenerImpl* controlPanelListener=0;
NotificationService* conService=0;
NotificationService* notificationService=0;
NotificationSender* notificationSender=0;
ControllerNotificationReceiver* controller_receiver=0;

BusAttachment* bus;


//Component/property/CID/t/time/type
typedef tuple <string, string, string, int, time_t, string> muzzley_req;
vector <muzzley_req> req_vec;

//component/label/status/voltage/current/freq/watt/accu/GetProperties/On/Off/time
typedef tuple <string, string, Property*, Property*, Property*, Property*, Property*, Property*, Action*, Action*, Action*, time_t> alljoyn_plug;
vector <alljoyn_plug> plug_vec;

//lampID/lampName
unordered_map <string, string> muzzley_lamplist;

muzzley::Client _muzzley_plugs_client;

void lsf_controller_service_print(){
    cout << endl << "ALLJOYN LSF CONTROLLER SERVICE INFO:" << endl << flush;
    cout << "CONTROLLER SERVICE CONNECTED: " << muzzley_controllerservice_connected << endl << flush;
    cout << "CONTROLLER SERVICE ID: " << muzzley_controllerservice_id << endl << flush;
    cout << "CONTROLLER SERVICE NAME: " << muzzley_controllerservice_name << endl << endl << flush;
}

string get_iface_macAdress(string ifc){
    int fd;
    struct ifreq ifr;
    const char *iface = ifc.c_str();
    unsigned char *mac;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
    ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);
         
    mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    //printf("Mac : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    char buffer [25];
    sprintf (buffer, "%.2x%.2x%.2x%.2x%.2x%.2x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return string(buffer);        
}

       
void alljoyn_execute_action(Action* action){
    QStatus status = action->executeAction();
    std::cout << "Action: " << action->getWidgetName().c_str() << (status == ER_OK ? " executed successfullly" : " failed") << std::endl;
}

//Color convert Utils
long long color_remap_long_long(long long x, long long in_min, long long in_max, long long out_min, long long out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double color_remap_double(double x, double in_min, double in_max, double out_min, double out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//Semaphore methods
void semaphore_lock(int& semaphore) {
    // Tests for '0' on the semaphore and increments '1' when '0' is observed
    struct sembuf lock[2] = { { (short unsigned int) 0, 0 }, { (short unsigned int) 0, 1 } };
    semop(semaphore, lock, 2);    
}

void semaphore_unlock(int& semaphore) {
    // Decrements '1' on the semaphore, unblocking threads waiting for '0'
    struct sembuf unlock[1] = { { (short unsigned int) 0, -1 } };
    semop(semaphore, unlock, 1);    
}

int semaphore_start(){
    key_t key = ftok(MUZZLEY_SEMAPHORE_FILENAME, 1);
    int semaphore = semget(key, 1, IPC_CREAT | 0777);
    if (semaphore == -1)
        cout << "Error on semaphore" << endl << flush;
    return semaphore;
}

void semaphore_stop(int sig) {
    //Destroy the Semaphore
    key_t key = ftok(MUZZLEY_SEMAPHORE_FILENAME, 1); // connects to the same semaphore
    int semaphore = semget(key, 1, IPC_CREAT | 0777);
    semctl(semaphore, 0, IPC_RMID);
    cout << "Exiting publish semaphore..." << endl << flush;
    
    string s = MUZZLEY_SEMAPHORE_FILENAME;
    unlink (s.c_str());
    exit(0);
}

bool muzzley_check_semaphore_file_exists(string filename){
    char s[200];
    sprintf(s, "test -e %s", filename.c_str());
    if (system(s) == 0)
        return true;
    else
        return false;
}

void muzzley_write_semaphore_file(){
    ofstream myfile;
    myfile.open (MUZZLEY_SEMAPHORE_FILENAME);    
    myfile << "muzzley_connector_running";
    myfile.close();
}

string muzzley_read_lighting_deviceKey_file(){
    ifstream myfile;
    string line;
    myfile.open (MUZZLEY_LIGHTING_DEVICEKEY_FILENAME);
    getline(myfile, line);
    myfile.close();
    return line;    
}

void muzzley_write_lighting_deviceKey_file(string deviceKey){
    ofstream myfile;
    myfile.open (MUZZLEY_LIGHTING_DEVICEKEY_FILENAME);    
    myfile << deviceKey;
    myfile.close();
}

string muzzley_read_plugs_deviceKey_file(){
    ifstream myfile;
    string line;
    myfile.open (MUZZLEY_PLUGS_DEVICEKEY_FILENAME);
    getline(myfile, line);
    myfile.close();
    return line;    
}

void muzzley_write_plugs_deviceKey_file(string deviceKey){
    ofstream myfile;
    myfile.open (MUZZLEY_PLUGS_DEVICEKEY_FILENAME);    
    myfile << deviceKey;
    myfile.close();
}

void RGBtoHSV( double r, double g, double b, double *h, double *s, double *v )
{
    /*
    Color Convertion
    using namespace std;
    r,g,b values are from 0 to 1
    h = [0,360], s = [0,1], v = [0,1]
    if (s == 0), then h = -1 (undefined)
    */
    
    double min, max, delta;
    min = std::min(r, g);
    min = std::min(min,b);
    max = std::max(r, g);
    max = std::max(max, b);

    *v = max;                   // v
    delta = max - min;
    if( max != 0 )
        *s = delta / max;       // s
    else {
        // r = g = b = 0        // s = 0, v is undefined
        *s = 0;
        *h = -1;
        return;
    }
    if( r == max )
        *h = ( g - b ) / delta;        // between yellow & magenta
    else if( g == max )
        *h = 2 + ( b - r ) / delta;    // between cyan & yellow
    else
        *h = 4 + ( r - g ) / delta;    // between magenta & cyan
    *h *= 60;                        // degrees
    if( *h < 0 )
        *h += 360;
}

void HSVtoRGB( double *r, double *g, double *b, double h, double s, double v )
{
    int i;
    double f, p, q, t;
    
    if(s==0) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }
    
    h = h/60;
    i = floor(h);
    f = h-i;            // factorial part of h

    p = v*(1-s);
    q = v*(1-s*f);
    t = v*(1-s*(1-f));
    
    switch(i) {
        case 0:
            *r = v; *g = t; *b = p;
            break;
        case 1:
            *r = q; *g = v; *b = p;
            break;
        case 2:
            *r = p; *g = v; *b = t;
            break;
        case 3:
            *r = p; *g = q; *b = v;
            break;
        case 4:
            *r = t; *g = p; *b = v;
            break;
        default: // case 5
            *r = v; *g = p; *b = q;
            break;
    }
}

int get_plug_vector_pos(string component){
    try{
        for (unsigned int i = 0; i < plug_vec.size(); i++){
            if(get<0>(plug_vec[i])==component){
                cout << "Plug found on pos#: " << i << endl << flush; 
                return i;
            }
        }
        cout << "Plug not found" << endl << flush; 
        return -1;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }
}

bool add_plug_vector_pos(string device_id_str, string device_name_str, Property* plug_property_status, Property* plug_property_volt, Property* plug_property_curr, Property* plug_property_freq, Property* plug_property_watt, Property* plug_property_accu, Action* plug_action_get_properties, Action* plug_action_on, Action* plug_action_off){
    try{
        time_t now = std::time(0);
        plug_vec.push_back(make_tuple(device_id_str, device_name_str, plug_property_status, plug_property_volt, plug_property_curr, plug_property_freq, plug_property_watt , plug_property_accu, plug_action_get_properties, plug_action_on, plug_action_off, now));
        cout << "Stored new plug info sucessfully" << endl << flush; 
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool del_plug_vector_pos(string component){
    try{
        for (unsigned int i = 0; i < plug_vec.size(); i++){
            if(get<0>(plug_vec[i])==component){
                plug_vec.erase(plug_vec.begin()+i);
                cout << "Deleted plug on pos#: " << i << endl << flush; 
                return true;
            }
        }
        cout << "Plug not found" << endl << flush; 
        return false;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

string get_plug_vector_componentID(int i){
    try{        
        return get<0>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

string get_plug_vector_label(int i){
    try{        
        cout << "Component: " << get<0>(plug_vec[i]) << endl << flush;
        cout << "Label: " << get<1>(plug_vec[i]) << endl << flush;
        return get<1>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

Property* get_plug_vector_property_status(int i){
    try{        
        return get<2>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Property* get_plug_vector_property_voltage(int i){
    try{        
        return get<3>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Property* get_plug_vector_property_current(int i){
    try{        
        return get<4>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Property* get_plug_vector_property_frequency(int i){
    try{
        return get<5>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Property* get_plug_vector_property_power(int i){
    try{
        return get<6>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Property* get_plug_vector_property_energy(int i){
    try{        
        return get<7>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Action* get_plug_vector_action_getproperties(int i){
    try{        
        return get<8>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Action* get_plug_vector_action_set_on(int i){
    try{        
        return get<9>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

Action* get_plug_vector_action_set_off(int i){
    try{        
        return get<10>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return NULL;
    }
}

time_t get_plug_vector_time(int i){
    try{        
        return get<11>(plug_vec[i]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return 0;
    }
}

bool muzzley_plug_vector_check(string component){
    try{
        for (unsigned int i = 0; i < plug_vec.size(); i++){
            if(strcmp(get<0>(plug_vec[i]).c_str(), component.c_str())){
                return true;
            }
        }
        return false;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
    }
}


void muzzley_plug_vector_print(){
    try{
        cout << endl << "PlugList:" << endl << flush;
        for (unsigned int i = 0; i < plug_vec.size(); i++){
            cout << endl << "Pos#: " << i+1 << "/" << plug_vec.size() << endl << flush;
            cout << "id: " << get<0>(plug_vec[i]) << " Name: " << get<1>(plug_vec[i]) << endl << flush;
        }
        cout << "---END---" << endl << endl << flush;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
    }
}


void print_request_pos(int pos){
    double req_duration = difftime(time(0), get<4>(req_vec[pos]));
    cout << "Request#: " << pos+1 << "/" << req_vec.size() << endl << flush;
    cout << "Component: " << get<0>(req_vec[pos]) << endl << flush;
    cout << "Property: " << get<1>(req_vec[pos]) << endl << flush;
    cout << "CID: " << get<2>(req_vec[pos]) << endl << flush;
    cout << "T: " << get<3>(req_vec[pos]) << endl << flush;
    cout << "Time: " << get<4>(req_vec[pos]) << endl << flush;
    cout << "Type: " << get<5>(req_vec[pos]) << endl << flush;
    cout << "Lived: " << req_duration << " sec" << endl << endl << flush;           
}

void print_request_vector(){
    cout << endl << "---Muzzley Read Requests:---" << endl << endl << flush;
    for (unsigned int i = 0; i < req_vec.size(); i++){
        print_request_pos(i);
    }
    cout << "---END---" << endl << endl << flush;
}

int get_request_vector_pos(string component, string property){
    try{
        for (unsigned int i = 0; i < req_vec.size(); i++){
            if(get<0>(req_vec[i])==component){
                if(get<1>(req_vec[i])==property){
                    cout << "Muzzley pending request found on pos#: " << i << endl << flush; 
                    return i;
                }
            }
        }
        cout << "Muzzley pending request not found" << endl << flush; 
        return -1;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }
}

string get_request_vector_CID(int pos){
    try{
        return get<2>(req_vec[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }    
}

int get_request_vector_t(int pos){
    try{
        return get<3>(req_vec[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }    
}

bool delete_request_vector_pos(int pos){
    try{
        req_vec.erase(req_vec.begin()+pos);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }    
}

bool muzzley_clean_request_vector(){
    for (unsigned int i = 0; i < req_vec.size(); i++){
        double req_duration = difftime(time(0), get<4>(req_vec[i]));
        if(req_duration>MUZZLEY_READ_REQUEST_TIMEOUT){
            cout << endl << "Erased muzzley read request (timeout):" << endl << flush;
            print_request_pos(i);
            req_vec.erase(req_vec.begin()+i);
        }
    }
    return true;
}

bool muzzley_query_unknown_lampnames(LampManager* lampManager){
    int status;
    for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(local_it->second ==  MUZZLEY_UNKNOWN_NAME){
                cout << endl << "Quering unknown lamp name for id: " << local_it->first << endl << flush;
                status = lampManager->GetLampName(local_it->first);
                if (status == LSF_ERR_FAILURE){
                    return false;
                }
            }
        }
    }
    return true;
}


bool muzzley_lamplist_update_lampname(string lampID, string lampName){
    for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(local_it->first == lampID){
                cout << endl << "Updating lamp name for id: " << local_it->first << endl << flush;
                local_it->second=lampName;
                return true;
            }
        }
    }
    muzzley_lamplist[lampID] = lampName;
    cout << endl << "Added new lamp info for id: " << lampID << endl << flush;
    return true;
}

string muzzley_lamplist_get_lampname(string lampID){
    for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(local_it->first == lampID.data()){
                //cout << endl << "Found lamp:" << endl << flush;
                if(local_it->second != MUZZLEY_UNKNOWN_NAME){
                    //cout << "id: " << local_it->first << " Name: " << local_it->second << endl << flush;
                    return local_it->second;
                }
            }
        }
    }
    return MUZZLEY_UNKNOWN_NAME;
}

bool muzzley_lamplist_del_lamp(string lampID){
   for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(strcmp(local_it->first.c_str(), lampID.c_str())==0){
                muzzley_lamplist.erase(lampID);
                return true;
            }
        }
    }
    return false;
}

bool muzzley_lamplist_check_lamp(string lampID){
   for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(strcmp(local_it->first.c_str(), lampID.c_str())==0){
                return true;
            }
        }
    }
    return false;
}

void muzzley_lamplist_print(){
    cout << endl << "Lamplist: " << endl << flush;
    for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            cout << "id: " << local_it->first << " Name: " << local_it->second << endl << flush;
        }
    }
    cout << "---END---" << endl << endl << flush;
}

void gupnp_generate_lighting_XML(){

        std::stringstream responseStream;

        responseStream << "<?xml version=\"1.0\"?>\n";
        responseStream << "<root>\n";
        responseStream << "<specVersion>\n";
        responseStream << "<major>1</major>\n";
        responseStream << "<minor>0</minor>\n";
        responseStream << "</specVersion>\n";
        responseStream << "<device>\n";
        responseStream << "<deviceType>urn:Muzzley:device:" << muzzley_lighting_profileid << ":1</deviceType>\n";
        responseStream << "<friendlyName>" << muzzley_lighting_upnp_friendlyname << "</friendlyName>\n";
        responseStream << "<manufacturer>" << muzzley_manufacturer << "</manufacturer>\n";
        responseStream << "<manufacturerURL>" << muzzley_manufacturer_url << "</manufacturerURL>\n";
        responseStream << "<modelDescription>" << muzzley_modeldescription << "</modelDescription>\n";
        responseStream << "<modelName>" << muzzley_modelname << "</modelName>\n";
        responseStream << "<modelNumber>" << muzzley_modelnumber << "</modelNumber>\n";
        responseStream << "<UDN>uuid:" << muzzley_lighting_upnp_udn << "</UDN>\n";
        responseStream << "<serialNumber>" << muzzley_lighting_upnp_serialnumber << "</serialNumber>\n";
        responseStream << "<macAddress>" << muzzley_lighting_macAddress << "</macAddress>\n";
        responseStream << "<deviceKey>" << muzzley_lighting_deviceKey << "</deviceKey>\n";
        responseStream << "<components>\n";
            if(MUZZLEY_BRIDGE_INFO){
                if(muzzley_controllerservice_id!=""){
                responseStream << "<component>\n";
                    responseStream << "<id>";
                    responseStream << muzzley_controllerservice_id;
                    responseStream << "</id>\n";

                    responseStream << "<label>";
                    responseStream << muzzley_controllerservice_name;
                    responseStream << "</label>\n";

                    responseStream << "<type>";
                    responseStream << DEVICE_BRIDGE;
                    responseStream << "</type>\n";
                responseStream << "</component>\n";
                }
            }
            
            for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
                for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
                    responseStream << "<component>\n";
                    responseStream << "<id>";
                    responseStream << local_it->first;
                    responseStream << "</id>\n";

                    responseStream << "<label>";
                    responseStream << muzzley_lamplist[local_it->first];
                    responseStream << "</label>\n";

                    responseStream << "<type>";
                    responseStream << DEVICE_BULB;
                    responseStream << "</type>\n";
                    responseStream << "</component>\n";
                }
            }
        responseStream << "</components>\n";
        responseStream << "</device>\n";
        responseStream << "</root>\n";

        ofstream myfile;
        myfile.open (MUZZLEY_LIGHTING_XML_FILENAME);
        const std::string tmp = responseStream.str();
        const char* str_xml = tmp.c_str();    
        myfile << str_xml;
        myfile.close();
    }

bool muzzley_lighting_connect_API(){

    // Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(muzzley_api_endpointhost, muzzley_api_port); //HTTPS:443

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPGet);

    // set HTTP request server path
    std::stringstream url;
    url << "/profiles/" << muzzley_lighting_profileid;
    _req->url(url.str());
    _req->header("Host", muzzley_api_endpointhost);
    _req->header("Accept", "*/*");

    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;

    _socket.close();

    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _url = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << "Parsed Muzzley API Reply:" << endl << flush;
        cout << "id: " << (string)_url["id"] << endl << flush;
        cout << "uuid: " << (string)_url["uuid"] << endl << flush;
        cout << "name: " << (string)_url["name"] << endl << flush;
        cout << "provider: " << (string)_url["provider"] << endl << flush;
        cout << "deviceHandlerUrl: " << (string)_url["deviceHandlerUrl"] << endl << endl << flush;
        string str_url=(string)_url["deviceHandlerUrl"];

    }else{
        cout << "Error: " << _rep->status() << endl << flush; 
        return false;
    }

    return true;
}

bool muzzley_lighting_connect_manager(){

    // Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);

    muzzley_lighting_deviceKey=muzzley_read_lighting_deviceKey_file();
    cout << "File lighting device key: " << muzzley_lighting_deviceKey << endl << flush;

    // Instantiate a string with some body part
    muzzley::JSONObj _json_body_part;
        _json_body_part <<
            "profileId" << muzzley_lighting_profileid <<
            "macAddress" << muzzley_lighting_macAddress <<
            "serialNumber" << muzzley_lighting_upnp_serialnumber <<
            "friendlyName" << muzzley_lighting_upnp_friendlyname;

            if(muzzley_lighting_deviceKey!=""){
                _json_body_part <<
                "deviceKey" << muzzley_lighting_deviceKey;
            }

    string _str_body_part;
    _json_body_part->stringify(_str_body_part); 
    
    std::ostringstream ss;
    int lenght = _str_body_part.length();
    ss << lenght;

    // set HTTP request server path
    _req->url(muzzley_manager_register_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());

    _req->body(_str_body_part);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
    
    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _key = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << endl << "Parsed Global Manager Reply:" << endl << "Lighting deviceKey: " << (string)_key["deviceKey"] << endl << endl << flush;
        
        //Store deviceKey in a file
        muzzley_lighting_deviceKey = (string)_key["deviceKey"];
        muzzley_write_lighting_deviceKey_file(muzzley_lighting_deviceKey);
    }
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }

    _socket.close();

    muzzley_lighting_registered=true;

    return true;
}

bool muzzley_replace_lighting_components(){

    //Generate XML File
    gupnp_generate_lighting_XML();

    if(muzzley_lighting_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);


    muzzley::JSONArr _components;
    if(MUZZLEY_BRIDGE_INFO){
        muzzley::JSONObj _bridge = JSON(
            "id" << muzzley_controllerservice_id <<
            "label" << muzzley_controllerservice_name <<
            "type" << "bridge"
        );
    _components << _bridge;
    }
    

    LSFStringList::const_iterator it = lampList.begin();
    for (; it != lampList.end(); ++it) {
        muzzley::JSONObj _bulb = JSON(
                "id" <<  (string)(*it).data() <<
                "label" << muzzley_lamplist[(*it).data()] <<
                "type" << "bulb"
            );
            _components << _bulb;
    }


    // Instantiate a string with some body part
    muzzley::JSONObj _replace_components;
        _replace_components <<
            "components" << _components;

    string _str_replace_components;
    _replace_components->stringify(_str_replace_components);
       
    std::ostringstream ss;
    int lenght = _str_replace_components.length();
    ss << lenght;

    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_replace_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

bool muzzley_add_lighting_component(LSFString lampID, LSFString lampName){

    //Generate XML File
    gupnp_generate_lighting_XML();

    if(muzzley_lighting_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPut);


    muzzley::JSONArr _component;
    muzzley::JSONObj _bulb = JSON(
        "id" <<  lampID <<
        "label" << lampName <<
        "type" << "bulb"
    );
    _component << _bulb;


    // Instantiate a string with some body part
    muzzley::JSONObj _add_components;
        _add_components <<
            "components" << _component;

    string _str_add_component;
    _add_components->stringify(_str_add_component);
     
    std::ostringstream ss;
    int lenght = _str_add_component.length();
    ss << lenght;
       
    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);

    _req->body(_str_add_component);
    _socket << _req << flush;
    cout << endl <<_req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}


bool muzzley_add_lighting_components(LSFStringList new_lampIDs){

    //Generate XML File
    gupnp_generate_lighting_XML();

    if(muzzley_lighting_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPut);


    muzzley::JSONArr _components;
    LSFStringList::const_iterator it = new_lampIDs.begin();
    for (; it != new_lampIDs.end(); ++it) {
        muzzley::JSONObj _bulb = JSON(
                "id" <<  (string)(*it).data() <<
                "label" << muzzley_lamplist[(*it).data()] <<
                "type" << "bulb"
            );
            _components << _bulb;
    }

    // Instantiate a string with some body part
    muzzley::JSONObj _add_components;
        _add_components <<
            "components" << _components;

    string _str_add_components;
    _add_components->stringify(_str_add_components);
   
    std::ostringstream ss;
    int lenght = _str_add_components.length();
    ss << lenght;
      
    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_add_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

bool muzzley_remove_lighting_components(LSFStringList del_lampIDs){

    //Generate XML File
    gupnp_generate_lighting_XML();

    if(muzzley_lighting_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPDelete);


    muzzley::JSONArr _components;
    LSFStringList::const_iterator it = del_lampIDs.begin();
    for (; it != del_lampIDs.end(); ++it) {
        muzzley::JSONObj _bulb = JSON(
                "id" <<  (string)(*it).data()
            );
            _components << _bulb;
    }

    // Instantiate a string with some body part
    muzzley::JSONObj _del_components;
        _del_components <<
            "components" << _components;

    string _str_del_components;
    _del_components->stringify(_str_del_components);
   
    std::ostringstream ss;
    int lenght = _str_del_components.length();
    ss << lenght;

    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_del_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

void gupnp_generate_plugs_XML(){

    std::stringstream responseStream;

    responseStream << "<?xml version=\"1.0\"?>\n";
    responseStream << "<root>\n";
    responseStream << "<specVersion>\n";
    responseStream << "<major>1</major>\n";
    responseStream << "<minor>0</minor>\n";
    responseStream << "</specVersion>\n";
    responseStream << "<device>\n";
    responseStream << "<deviceType>urn:Muzzley:device:" << muzzley_plugs_profileid << ":1</deviceType>\n";
    responseStream << "<friendlyName>" << muzzley_plugs_upnp_friendlyname << "</friendlyName>\n";
    responseStream << "<manufacturer>" << muzzley_manufacturer << "</manufacturer>\n";
    responseStream << "<manufacturerURL>" << muzzley_manufacturer_url << "</manufacturerURL>\n";
    responseStream << "<modelDescription>" << muzzley_modeldescription << "</modelDescription>\n";
    responseStream << "<modelName>" << muzzley_modelname << "</modelName>\n";
    responseStream << "<modelNumber>" << muzzley_modelnumber << "</modelNumber>\n";
    responseStream << "<UDN>uuid:" << muzzley_plugs_upnp_udn << "</UDN>\n";
    responseStream << "<serialNumber>" << muzzley_plugs_upnp_serialnumber << "</serialNumber>\n";
    responseStream << "<macAddress>" << muzzley_plugs_macAddress << "</macAddress>\n";
    responseStream << "<deviceKey>" << muzzley_plugs_deviceKey << "</deviceKey>\n";
    responseStream << "<components>\n";
    try{
        for (unsigned int i = 0; i < plug_vec.size(); i++){
            if(get<0>(plug_vec[i])!=""){
                responseStream << "<component>\n";
                responseStream << "<id>" << get<0>(plug_vec[i]) << "</id>\n";
                responseStream << "<label>" << get<1>(plug_vec[i]) << "</label>\n";
                responseStream << "<type>" << DEVICE_PLUG << "</type>\n";
                responseStream << "</component>\n";
            }
        }
        
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
    }
    responseStream << "</components>\n";
    responseStream << "</device>\n";
    responseStream << "</root>\n";

    ofstream myfile;
    myfile.open (MUZZLEY_PLUGS_XML_FILENAME);
    const std::string tmp = responseStream.str();
    const char* str_xml = tmp.c_str();    
    myfile << str_xml;
    myfile.close();
}


bool muzzley_plugs_connect_API(){
    // Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(muzzley_api_endpointhost, muzzley_api_port); //HTTPS:443

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPGet);

    // set HTTP request server path
    std::stringstream url;
    url << "/profiles/" << muzzley_plugs_profileid;
    _req->url(url.str());
    _req->header("Host", muzzley_api_endpointhost);
    _req->header("Accept", "*/*");

    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;

    _socket.close();

    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _url = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << "Parsed Muzzley API Reply:" << endl << flush;
        cout << "id: " << (string)_url["id"] << endl << flush;
        cout << "uuid: " << (string)_url["uuid"] << endl << flush;
        cout << "name: " << (string)_url["name"] << endl << flush;
        cout << "provider: " << (string)_url["provider"] << endl << flush;
        cout << "deviceHandlerUrl: " << (string)_url["deviceHandlerUrl"] << endl << endl << flush;
        string str_url=(string)_url["deviceHandlerUrl"];

    }else{
        cout << "Error: " << _rep->status() << endl << flush; 
        return false;
    }

    return true;
}

bool muzzley_plugs_connect_manager(){

    // Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);

    muzzley_plugs_deviceKey=muzzley_read_plugs_deviceKey_file();
    cout << "File plugs device key: " << muzzley_plugs_deviceKey << endl << flush;

    // Instantiate a string with some body part
    muzzley::JSONObj _json_body_part;
        _json_body_part <<
            "profileId" << muzzley_plugs_profileid <<
            "macAddress" << muzzley_plugs_macAddress <<
            "serialNumber" << muzzley_plugs_upnp_serialnumber <<
            "friendlyName" << muzzley_plugs_upnp_friendlyname;

            if(muzzley_plugs_deviceKey!=""){
                _json_body_part <<
                "deviceKey" << muzzley_plugs_deviceKey;
            }

    string _str_body_part;
    _json_body_part->stringify(_str_body_part); 
    
    std::ostringstream ss;
    int lenght = _str_body_part.length();
    ss << lenght;

    // set HTTP request server path
    _req->url(muzzley_manager_register_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());

    _req->body(_str_body_part);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
    
    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _key = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << endl << "Parsed Global Manager Reply:" << endl << "Plugs deviceKey: " << (string)_key["deviceKey"] << endl << endl << flush;
        
        //Store deviceKey in a file
        muzzley_plugs_deviceKey = (string)_key["deviceKey"];
        muzzley_write_plugs_deviceKey_file(muzzley_plugs_deviceKey);
    }
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }

    _socket.close();

    muzzley_plugs_registered=true;

    return true;
}

bool muzzley_replace_plugs_components(){

    //Generate XML File
    gupnp_generate_plugs_XML();

    if(muzzley_plugs_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);


    muzzley::JSONArr _components;
    
    for (unsigned int i = 0; i < plug_vec.size(); i++){
        muzzley::JSONObj _bulb = JSON(
            "id" <<  get<0>(plug_vec[i]) <<
            "label" << get<1>(plug_vec[i]) <<
            "type" << DEVICE_PLUG
        );
        _components << _bulb;
    }

    // Instantiate a string with some body part
    muzzley::JSONObj _replace_components;
        _replace_components <<
            "components" << _components;

    string _str_replace_components;
    _replace_components->stringify(_str_replace_components);
       
    std::ostringstream ss;
    int lenght = _str_replace_components.length();
    ss << lenght;

    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_plugs_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_plugs_deviceKey);

    _req->body(_str_replace_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}
bool muzzley_add_plugs_component(string plug_id, string plug_name){

    //Generate XML File
    gupnp_generate_plugs_XML();

    if(muzzley_plugs_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPut);

    muzzley::JSONArr _component;
    muzzley::JSONObj _plug = JSON(
        "id" <<  plug_id <<
        "label" << plug_name <<
        "type" << DEVICE_PLUG
    );
    _component << _plug;


    // Instantiate a string with some body part
    muzzley::JSONObj _add_components;
        _add_components <<
            "components" << _component;

    string _str_add_component;
    _add_components->stringify(_str_add_component);
     
    std::ostringstream ss;
    int lenght = _str_add_component.length();
    ss << lenght;
       
    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);

    _req->body(_str_add_component);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

bool muzzley_add_plugs_components(string plug_id, string plug_name){

    //Generate XML File
    gupnp_generate_plugs_XML();

    if(muzzley_plugs_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPut);

    muzzley::JSONArr _components;
    muzzley::JSONObj _plug = JSON(
        "id" <<  plug_id <<
        "label" << plug_name <<
        "type" << DEVICE_PLUG
    );
    _components << _plug;

    // Instantiate a string with some body part
    muzzley::JSONObj _add_components;
        _add_components <<
            "components" << _components;

    string _str_add_components;
    _add_components->stringify(_str_add_components);
   
    std::ostringstream ss;
    int lenght = _str_add_components.length();
    ss << lenght;
      
    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_add_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

bool muzzley_remove_plugs_component(string plug_id, string plug_name){

    //Generate XML File
    gupnp_generate_plugs_XML();

    if(muzzley_lighting_registered==false){
        return false;
    }

    muzzley::socketstream _socket;
    _socket.open(muzzley_manager_endpointhost, muzzley_manager_port);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPDelete);


    muzzley::JSONArr _components;
    
    muzzley::JSONObj _plug = JSON(
        "id" <<  plug_id <<
        "label" << plug_name <<
        "type" << DEVICE_PLUG
    );
    _components << _plug;

    // Instantiate a string with some body part
    muzzley::JSONObj _del_components;
        _del_components <<
            "components" << _components;

    string _str_del_components;
    _del_components->stringify(_str_del_components);
   
    std::ostringstream ss;
    int lenght = _str_del_components.length();
    ss << lenght;

    //set HTTP request server path
    _req->url(muzzley_manager_components_url);
    _req->header("Host", muzzley_manager_endpointhost);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", muzzley_lighting_upnp_serialnumber);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_del_components);
    _socket << _req << flush;
    cout << endl << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;
        
    if (_rep->status() == muzzley::HTTP200) {}
    else{
        cout << "Error: " << _rep->status() << endl << flush;
    }
    _socket.close();
    return true;
}

bool muzzley_publish_lampState(LSFString lampID, bool onoff, muzzley::Client* _muzzley_lighting_client){
    try{
        
        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_STATUS);

        muzzley::Message _m1;
        int pos = get_request_vector_pos(lampID, PROPERTY_STATUS);
        
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
               "value" <<  onoff <<
               "profile" << muzzley_lighting_profileid <<
               "channel" << muzzley_lighting_deviceKey <<
               "component" << lampID <<
               "property" << PROPERTY_STATUS <<
               "data" << JSON(
                   "value" <<  onoff
                   )
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_lighting_client->reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed bulb status!" << endl << flush;
            return true;
        }
            
        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
               "value" <<  onoff
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_lighting_client->trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
    
}

bool muzzley_publish_brightness(LSFString lampID, double brightness, muzzley::Client* _muzzley_lighting_client){
    try{

        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_BRIGHTNESS);

        muzzley::Message _m1;

        int pos = get_request_vector_pos(lampID, PROPERTY_BRIGHTNESS);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
                "value" <<  brightness <<
                "profile" << muzzley_lighting_profileid <<
                "channel" << muzzley_lighting_deviceKey <<
                "component" << lampID <<
                "property" << PROPERTY_BRIGHTNESS <<
                "data" << JSON(
                    "value" <<  brightness
                    )
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_lighting_client->reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed bulb brightness!" << endl << flush;
            return true;
        }
         
       _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
                "value" <<  brightness
            )
        ));
        
        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_lighting_client->trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}



bool muzzley_publish_lampColor_rgb(LSFString lampID, int red, int green, int blue, muzzley::Client* _muzzley_lighting_client){
    try{

        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_COLOR_RGB);


        muzzley::Message _m1;

        int pos = get_request_vector_pos(lampID, PROPERTY_COLOR_RGB);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
                "value" << JSON(
                    "r" << red <<
                    "g" << green <<
                    "b" << blue
                      ) <<
                "profile" << muzzley_lighting_profileid <<
                "channel" << muzzley_lighting_deviceKey <<
                "component" << lampID <<
                "property" << PROPERTY_COLOR_RGB <<
                "data" << JSON(
                    "value" <<  JSON(
                        "r" << red <<
                        "g" << green <<
                        "b" << blue
                          )
                    ) 
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_lighting_client->reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed bulb color!" << endl << flush;
            return true;
        }
      
        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
                "value" << JSON(
                "r" << red <<
                "g" << green <<
                "b" << blue
                )    
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_lighting_client->trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_publish_lampColor_hsv(LSFString lampID, int hue, int saturation, int value, muzzley::Client* _muzzley_lighting_client){
    try{

        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_COLOR_HSV);


        muzzley::Message _m1;

        int pos = get_request_vector_pos(lampID, PROPERTY_COLOR_HSV);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
                "value" << JSON(
                    "h" << hue <<
                    "s" << saturation <<
                    "v" << value
                ) <<
                "profile" << muzzley_lighting_profileid <<
                "channel" << muzzley_lighting_deviceKey <<
                "component" << lampID <<
                "property" << PROPERTY_COLOR_HSV <<
                "data" << JSON(
                    "value" <<  JSON(
                        "h" << hue <<
                        "s" << saturation <<
                        "v" << value
                    )
                ) 
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_lighting_client->reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed bulb color hsv!" << endl << flush;
            return true;
        }
      
        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
                "value" << JSON(
                    "h" << hue <<
                    "s" << saturation <<
                    "v" << value
                )    
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_lighting_client->trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_publish_lampColor_hsvt(LSFString lampID, int hue, int saturation, int value, int temperature, muzzley::Client* _muzzley_lighting_client){
    try{

        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_COLOR_HSVT);


        muzzley::Message _m1;

        int pos = get_request_vector_pos(lampID, PROPERTY_COLOR_HSVT);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
                "value" << JSON(
                    "h" << hue <<
                    "s" << saturation <<
                    "v" << value <<
                    "t" << temperature
                ) <<
                "profile" << muzzley_lighting_profileid <<
                "channel" << muzzley_lighting_deviceKey <<
                "component" << lampID <<
                "property" << PROPERTY_COLOR_HSVT <<
                "data" << JSON(
                    "value" <<  JSON(
                        "h" << hue <<
                        "s" << saturation <<
                        "v" << value <<
                        "t" << temperature
                    )
                ) 
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_lighting_client->reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed bulb color hsvt!" << endl << flush;
            return true;
        }
      
        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
                "value" << JSON(
                    "h" << hue <<
                    "s" << saturation <<
                    "v" << value <<
                    "t" << temperature
                )    
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_lighting_client->trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_parseLampState(LSFString lampID, LampState lampState, muzzley::Client* _muzzley_lighting_client){

    try{
        std::string line, onoff, hue, saturation, brightness, colortemp;
        std::string input = lampState.c_str();
        std::string delimiter = "=";
        std::stringstream ss(input);

        while(std::getline(ss, line))
        {
            std::string key = line.substr(0, line.find(delimiter));
            std::string value = line.substr(line.find(delimiter)+1);
            if(key == "OnOff") {
                onoff = value;
            } else if(key == "Brightness") {
                brightness = value; 
            } else if(key == "Hue") {
                hue = value;
            } else if(key == "Saturation") {
                saturation = value;
            } else if(key == "ColorTemp") {
                colortemp = value;
            }
        }
        
        bool onoff_state = (bool) atoi(onoff.data());
        long long long_brightness = atoll (brightness.data());
        long long long_hue = atoll (hue.data());
        long long long_saturation = atoll (saturation.data());
        long long long_colortemp = atoll (colortemp.data());

        long long brightness_int = color_remap_long_long(long_brightness, COLOR_MIN, (COLOR_MAX_UINT32-1), COLOR_MIN,                 COLOR_MAX_PERCENT);
        long long hue_int        = color_remap_long_long(long_hue,        COLOR_MIN, (COLOR_MAX_UINT32-1), COLOR_MIN,                 COLOR_MAX_360DEG);
        long long saturation_int = color_remap_long_long(long_saturation, COLOR_MIN, (COLOR_MAX_UINT32-1), COLOR_MIN,                 COLOR_MAX_PERCENT);
        long long colortemp_int  = color_remap_long_long(long_colortemp,  COLOR_MIN, (COLOR_MAX_UINT32-1), COLOR_TEMPERATURE_MIN_DEC, COLOR_TEMPERATURE_MAX_DEC);

        if(strcmp(muzzley_color_mode.c_str(), PROPERTY_COLOR_HSV)==0){
        	if(muzzley_publish_lampColor_hsv(lampID, hue_int, saturation_int, brightness_int, _muzzley_lighting_client)==false){
            	cout  << "Error publishing lampcolorHSV to muzzley" << endl << flush;
	        }else{
	            cout  << "Published lampcolorHSV to muzzley sucessfully" << endl << flush;
	        }
        }
        
        if(strcmp(muzzley_color_mode.c_str(), PROPERTY_COLOR_HSVT)==0){
        	if(muzzley_publish_lampColor_hsvt(lampID, hue_int, saturation_int, brightness_int, colortemp_int, _muzzley_lighting_client)==false){
            	cout  << "Error publishing lampcolorHSVT to muzzley" << endl << flush;
	        }else{
	            cout  << "Published lampcolorHSVT to muzzley sucessfully" << endl << flush;
	        }
        }

        std::cout <<  "\n{\"onOff\": " << onoff << "," << std::endl;
        std::cout <<  "\"brightness\": " << brightness << "," << std::endl;
        std::cout <<  "\"hue\": " << hue << "," << std::endl;
        std::cout <<  "\"saturation\": " << saturation << "," << std::endl;
        std::cout <<  "\"colorTemp\": " << colortemp << "}" << std::endl;

        printf("\nonOff: %s\n", (onoff_state)?"true":"false");
        printf("brightness: %lld\n", long_brightness);
        printf("hue: %lld\n", long_hue);
        printf("saturation: %lld\n", long_saturation);
        printf("colorTemp: %lld\n", long_colortemp);
        printf("Brightness: %lld\n", brightness_int);
        printf("Hue: %lld\n", hue_int);
        printf("Saturation: %lld\n", saturation_int);
        printf("ColorTemp: %lld\n\n", colortemp_int);


        if(muzzley_publish_lampState(lampID, onoff_state, _muzzley_lighting_client)==false){
            cout << "Error publishing lamponoffstate to muzzley" << endl << flush;
        }else{
            cout << "Published lamponoffstate to muzzley sucessfully" << endl << flush;
        }
       
        double value_double = color_remap_double(brightness_int, COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, 1);
       
        if(muzzley_publish_brightness(lampID, value_double, _muzzley_lighting_client)==false){
            cout  << "Error publishing brightness to muzzley" << endl << flush;
        }else{
            cout  << "Published brightness to muzzley sucessfully" << endl << flush;
        }

        double        hue_double = hue_int;
        double saturation_double = color_remap_double(saturation_int, COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, COLOR_DOUBLE_MAX);
                    value_double = color_remap_double(brightness_int, COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, COLOR_DOUBLE_MAX);


        double red_double, green_double, blue_double;
        HSVtoRGB(&red_double, &green_double, &blue_double, hue_double, saturation_double, value_double);

        int red   = (int)color_remap_double(red_double,   COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, COLOR_RGB_MAX);
        int green = (int)color_remap_double(green_double, COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, COLOR_RGB_MAX);
        int blue  = (int)color_remap_double(blue_double,  COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, COLOR_RGB_MAX);

        printf("\nHue: %f\n", hue_double);
        printf("Saturation: %f\n", saturation_double);
        printf("Brightness: %f\n", value_double);
        printf("Red: %f\n", red_double);
        printf("Green: %f\n", green_double);
        printf("Blue: %f\n", blue_double);  
        printf("Red: %d\n", red);
        printf("Green: %d\n", green);
        printf("Blue: %d\n\n", blue);
        
        if(strcmp(muzzley_color_mode.c_str(), PROPERTY_COLOR_RGB)==0){
        	if(muzzley_publish_lampColor_rgb(lampID, red, green, blue, _muzzley_lighting_client)==false){
            	cout  << "Error publishing lampcolorRGB to muzzley" << endl << flush;
	        }else{
	            cout  << "Published lampcolorRGB to muzzley sucessfully" << endl << flush;
	        }
        }
 
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_write_status_request(LampManager& lampManager, string component, bool bool_status){
    int status;
    try{
        status = lampManager.TransitionLampStateOnOffField(component, bool_status);
        if(status != LSF_OK)
            cout << "LampManager Error!" << endl << flush;
        if(status == 1)
            cout << "No lighting controller service running!" << endl << flush;
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_write_brightness_request(LampManager& lampManager, string component, double brightness){
    int status;
    try{
        brightness = color_remap_double(brightness, COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, (COLOR_MAX_UINT32-1));
        long long long_brightness = (long long) brightness;

        printf("Received brightness double: %f\n", brightness);
        printf("Received brightness long: %lld\n", long_brightness);

        status = lampManager.TransitionLampStateBrightnessField(component, long_brightness);
        if(status != LSF_OK)
            cout << "LampManager Error!" << endl << flush;
        if(status == 1)
            cout << "No lighting controller service running!" << endl << flush;
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_write_HSVT_request(LampManager& lampManager, string component, int hue, int saturation, int value, int colortemp){
    int status;
    try{
        
        double hue_double        = color_remap_double(hue,        COLOR_MIN,                  COLOR_MAX_360DEG,          COLOR_MIN,                 COLOR_DOUBLE_MAX);
        double saturation_double = color_remap_double(saturation, COLOR_MIN,                  COLOR_MAX_PERCENT,         COLOR_MIN,                 COLOR_DOUBLE_MAX);
        double value_double      = color_remap_double(value,      COLOR_MIN,                  COLOR_MAX_PERCENT,         COLOR_MIN,                 COLOR_DOUBLE_MAX);
        double colortemp_double  = color_remap_double(colortemp,  COLOR_TEMPERATURE_MIN_DEC,  COLOR_TEMPERATURE_MAX_DEC, COLOR_TEMPERATURE_MIN_DEC, COLOR_TEMPERATURE_MAX_DEC);
        

        long long long_hue        = color_remap_long_long(round((hue_double*100)),        COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, (COLOR_MAX_UINT32-1));
        long long long_saturation = color_remap_long_long(round((saturation_double*100)), COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, (COLOR_MAX_UINT32-1));
        long long long_brightness = color_remap_long_long(round((value_double*100)),      COLOR_MIN, COLOR_MAX_PERCENT, COLOR_MIN, (COLOR_MAX_UINT32-1));
        long long long_colortemp  = color_remap_long_long(colortemp_double,               COLOR_TEMPERATURE_MIN_DEC,    COLOR_TEMPERATURE_MAX_DEC, COLOR_MIN, (COLOR_MAX_UINT32-1));

        printf("Calculated hue double: %f\n", hue_double);
        printf("Calculated saturation double: %f\n", saturation_double);
        printf("Calculated value double: %f\n", value_double);
        printf("Calculated colortemp double: %f\n", colortemp_double);
        printf("Brightness: %lld\n", long_brightness);
        printf("Hue: %lld\n", long_hue);
        printf("Saturation: %lld\n", long_saturation);
        printf("ColorTemp: %lld\n", long_colortemp);
   
        //onoff/Hue/Saturation/Colortemp/Brightness
        LampState state(true, long_hue, long_saturation, long_colortemp, long_brightness);
        status = lampManager.TransitionLampState(component, state);
        if(status != LSF_OK)
            cout << "LampManager Error!" << endl << flush;
        if(status == 1)
            cout << "No lighting controller service running!" << endl << flush;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_write_colorname_request(LampManager& lampManager, string component, string colorname){
    try{
        int hue;
        int saturation;
        int value;

        if(strcmp(colorname.c_str(), COLOR_PURPLE)==0){
            hue=285;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_FUCHSIA)==0){
            hue=300;
            saturation=53;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_NAVY)==0){
            hue=210;
            saturation=88;
            value=82;
        }else if(strcmp(colorname.c_str(), COLOR_BLUE)==0){
            hue=215;
            saturation=100;
            value=67;
        }else if(strcmp(colorname.c_str(), COLOR_TEAL)==0){
            hue=180;
            saturation=100;
            value=50;
        }else if(strcmp(colorname.c_str(), COLOR_AQUA)==0){
            hue=180;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_GREEN)==0){
            hue=120;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_LIME)==0){
            hue=75;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_OLIVE)==0){
            hue=60;
            saturation=100;
            value=50;
        }else if(strcmp(colorname.c_str(), COLOR_YELLOW)==0){
            hue=60;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_MAROON)==0){
            hue=0;
            saturation=100;
            value=50;
        }else if(strcmp(colorname.c_str(), COLOR_RED)==0){
            hue=2;
            saturation=100;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_SILVER)==0){
            hue=0;
            saturation=0;
            value=75;
        }else if(strcmp(colorname.c_str(), COLOR_GRAY)==0){
            hue=0;
            saturation=0;
            value=66;
        }else if(strcmp(colorname.c_str(), COLOR_WHITE)==0){
            hue=0;
            saturation=0;
            value=100;
        }else if(strcmp(colorname.c_str(), COLOR_BLACK)==0){
            hue=0;
            saturation=0;
            value=0;
        }

        if(muzzley_handle_lighting_write_HSVT_request(lampManager, component, hue, saturation, value, COLOR_TEMPERATURE_DEFAULT_DEC))
            return true;
        else
            return false; 
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_read_request(LampManager& lampManager, string component, string property, string cid, int t){
    int status;
    try{
        time_t tm = std::time(0);
        req_vec.push_back(make_tuple(component, property, cid, t, tm, DEVICE_BULB));
        status = lampManager.GetLampState(component);
        if(status != LSF_OK)
            cout << "LampManager Error!" << endl << flush;
        if(status == 1)
            cout << "No lighting controller service running!" << endl << flush;
        muzzley_clean_request_vector();
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool muzzley_handle_lighting_request(LampManager& lampManager, muzzley::JSONObjT _data){
    string io = (string)_data["d"]["p"]["io"];
    string component =  (string)_data["d"]["p"]["component"];
    string property = (string)_data["d"]["p"]["property"];
    string user_name = (string)_data["d"]["u"]["name"];
    string user_id = (string)_data["d"]["u"]["profileId"];
    bool isOnBehalfOf = (bool)_data["d"]["u"]["isOnBehalfOf"];
    string cid = (string)_data["h"]["cid"];
    string ch = (string)_data["h"]["ch"];
    int t = (int)_data["h"]["t"];

    if(!muzzley_OnBehalfOf){
        if(isOnBehalfOf){
            cout << "Ignoring request on behalf of user id: " << user_id << " Name: " << user_name << endl << flush;
            return false;
        }
    }

    if(!muzzley_lamplist_check_lamp(component)){
        cout << "Received request for unknown lamp id: " << component << " from iser id: " << user_id << " Name: " << user_name << endl << flush;
        return false;
    }

    muzzley_query_unknown_lampnames(&lampManager);
    print_request_vector();

    if (io=="r"){
        muzzley_handle_lighting_read_request(lampManager, component, property, cid, t);
    }
    if (io=="w"){
        if(property==PROPERTY_STATUS){
            bool bool_status = (bool)_data["d"]["p"]["data"]["value"];
            muzzley_handle_lighting_write_status_request(lampManager, component, bool_status);
        }
        if(property==PROPERTY_BRIGHTNESS){
            double brightness = (double)_data["d"]["p"]["data"]["value"];
            muzzley_handle_lighting_write_brightness_request(lampManager, component, brightness);   
        }
        if(property==PROPERTY_COLOR_RGB){
            int red   = (int)_data["d"]["p"]["data"]["value"]["r"];
            int green = (int)_data["d"]["p"]["data"]["value"]["g"];
            int blue  = (int)_data["d"]["p"]["data"]["value"]["b"];

            double red_double   = color_remap_double(red,   COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);
            double green_double = color_remap_double(green, COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);
            double blue_double  = color_remap_double(blue,  COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);
            double hue_double, saturation_double, value_double;
            RGBtoHSV(red_double, green_double, blue_double, &hue_double, &saturation_double, &value_double);

            muzzley_handle_lighting_write_HSVT_request(lampManager, component, (int)(hue_double), (int)(saturation_double*100), (int)(value_double*100), COLOR_TEMPERATURE_DEFAULT_DEC);                                
        }
        if(property==PROPERTY_COLOR_HSV){
            int hue        = (int)_data["d"]["p"]["data"]["value"]["h"];
            int saturation = (int)_data["d"]["p"]["data"]["value"]["s"];
            int value      = (int)_data["d"]["p"]["data"]["value"]["v"];
            muzzley_handle_lighting_write_HSVT_request(lampManager, component, hue, saturation, value, COLOR_TEMPERATURE_DEFAULT_DEC);
        }
        if(property==PROPERTY_COLOR_HSVT){
            int hue        = (int)_data["d"]["p"]["data"]["value"]["h"];
            int saturation = (int)_data["d"]["p"]["data"]["value"]["s"];
            int value      = (int)_data["d"]["p"]["data"]["value"]["v"];
            int colortemp  = (int)_data["d"]["p"]["data"]["value"]["t"];
            muzzley_handle_lighting_write_HSVT_request(lampManager, component, hue, saturation, value, colortemp);
        }
        if(property==PROPERTY_COLOR_NAME){
            string colorname = (string)_data["d"]["p"]["data"]["value"];
            cout << "Requested color name: " << colorname << endl << flush;
            muzzley_handle_lighting_write_colorname_request(lampManager, component, colorname);  
        }
    }
    return true;
}



bool muzzley_publish_plug_state(string plugID, bool onoff){
    try{

        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_plugs_profileid);
        _s1.setChannel(muzzley_plugs_deviceKey);
        _s1.setComponent(plugID);
        _s1.setProperty(PROPERTY_STATUS);

        muzzley::Message _m1;

        int pos = get_request_vector_pos(plugID, PROPERTY_STATUS);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
               "value" <<  onoff <<
               "profile" << muzzley_plugs_profileid <<
               "channel" << muzzley_plugs_deviceKey <<
               "component" << plugID <<
               "property" << PROPERTY_STATUS <<
               "data" << JSON(
                   "value" <<  onoff
                   )
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_plugs_client.reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed plug status!" << endl << flush;
            return true;
        }

        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
               "value" <<  onoff
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_plugs_client.trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }    
}

bool muzzley_publish_plug_string(string plugID, string property, string value){
    try{
        
        int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_plugs_profileid);
        _s1.setChannel(muzzley_plugs_deviceKey);
        _s1.setComponent(plugID);
        _s1.setProperty(property);

        muzzley::Message _m1;

        int pos = get_request_vector_pos(plugID, property);
        if(pos!=-1){
            _m1.setStatus(true);
            _m1.setCorrelationID(get_request_vector_CID(pos));
            _m1.setMessageType((muzzley::MessageType)get_request_vector_t(pos));
            _m1.setData(JSON(
               "value" <<  value <<
               "profile" << muzzley_plugs_profileid <<
               "channel" << muzzley_plugs_deviceKey <<
               "component" << plugID <<
               "property" << property <<
               "data" << JSON(
                   "value" <<  value
                   )
            ));

            //cout << _s1 << endl << flush;
            //cout << _m1 << endl << flush;
            semaphore_lock(semaphore);
            _muzzley_plugs_client.reply(_m1, _m1);
            semaphore_unlock(semaphore);
            delete_request_vector_pos(pos);
            cout << "Replyed plug " << property << " string!" << endl << flush;
            return true;
        }

        _m1.setData(JSON(
            "io" << "i" <<
            "data" << JSON(
               "value" <<  value
            )
        ));

        //cout << _s1 << endl << flush;
        //cout << _m1 << endl << flush;
        semaphore_lock(semaphore);
        _muzzley_plugs_client.trigger(muzzley::Publish, _s1, _m1);
        semaphore_unlock(semaphore);
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }    
}

void muzzley_handle_plug_read_status_request(string component, string cid, int t){
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, PROPERTY_STATUS, cid, t, tm, DEVICE_PLUG));

    int pos = get_plug_vector_pos(component);
    Property* status_property = get_plug_vector_property_status(pos);

    if(status_property==NULL){
        return;
    }else{
        const char* status = status_property->getPropertyValue().charValue;
        cout << "Plug Status: " << status << endl << flush;
        bool onoff;
        
        if(strcmp(status, "Switch On")==0)
            onoff=true;
        else
            onoff=false;
        
        muzzley_publish_plug_state(component, onoff);
    }

}

void muzzley_handle_plug_read_voltage_request(string component, string cid, int t){
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, PROPERTY_VOLTAGE, cid, t, tm, DEVICE_PLUG));
    
    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* voltage_property = get_plug_vector_property_voltage(pos);

    if(voltage_property==NULL){
        cout << "Voltage not found" << endl << flush; 
    }else{
        const char* voltage = voltage_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_VOLTAGE, voltage);
        muzzley_clean_request_vector();
    }
}

void muzzley_handle_plug_read_current_request(string component, string cid, int t){
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, PROPERTY_CURRENT, cid, t, tm, DEVICE_PLUG));

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* current_property = get_plug_vector_property_current(pos);

    if(current_property==NULL){
        cout << "Current not found" << endl << flush; 
    }else{
        const char* current = current_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_CURRENT, current);
        muzzley_clean_request_vector();
    }
}

void muzzley_handle_plug_read_frequency_request(string component, string cid, int t){
    string property = PROPERTY_FREQUENCY;
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, property, cid, t, tm, DEVICE_PLUG));

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* frequency_property = get_plug_vector_property_frequency(pos);

    if(frequency_property==NULL){
        cout << "Frequency not found" << endl << flush; 
    }else{
        const char* frequency = frequency_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, property, frequency);
        muzzley_clean_request_vector();
    }
}

void muzzley_handle_plug_read_power_request(string component, string cid, int t){
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, PROPERTY_POWER, cid, t, tm, DEVICE_PLUG));

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* power_property = get_plug_vector_property_power(pos);

    if(power_property==NULL){
        cout << "Power not found" << endl << flush; 
    }else{
        const char* energy = power_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_POWER, energy);
        muzzley_clean_request_vector();
    }
}

void muzzley_handle_plug_read_energy_request(string component, string cid, int t){
    time_t tm = std::time(0);
    req_vec.push_back(make_tuple(component, PROPERTY_ENERGY, cid, t, tm, DEVICE_PLUG));

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* energy_property = get_plug_vector_property_energy(pos);

    if(energy_property==NULL){
        cout << "Energy not found" << endl << flush; 
    }else{
        const char* energy = energy_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_ENERGY, energy);
        muzzley_clean_request_vector();
    }
}


void muzzley_handle_plug_write_status_request(string component, bool bool_status){

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    Property* status_property = get_plug_vector_property_status(pos);
    Action* seton_action = get_plug_vector_action_set_on(pos);
    Action* setoff_action = get_plug_vector_action_set_off(pos);

    if(status_property==NULL){
        cout << "Status not found" << endl << flush; 
    }else{
        if(bool_status){
            alljoyn_execute_action(seton_action);
            //muzzley_publish_plug_state(component, true);                        
        }
        else{  
            alljoyn_execute_action(setoff_action);
            //muzzley_publish_plug_state(component, false);
          }
    }
}

void muzzley_update_plug_properties(string component){

    int pos = get_plug_vector_pos(component);
    if(pos==-1)
        return;

    /*
    Action* getp_action = get_plug_vector_action_getproperties(pos);
    if(getp_action==NULL)
        cout << "Get Properties action not found" << endl << flush; 
    else{
        sleep(2);
        alljoyn_execute_action(getp_action);
    }
    */

    /*
    double property_duration = difftime(time(0), get_plug_vector_time(pos));
    if(property_duration>2){
        Action* getp_action = get_plug_vector_action_getproperties(pos);
        if(getp_action==NULL)
            cout << "Get Properties action not found" << endl << flush; 
        else
            alljoyn_execute_action(getp_action);
    }
    */

    Action* getp_action = get_plug_vector_action_getproperties(pos);
    if(getp_action==NULL)
        cout << "Get Properties action not found" << endl << flush; 
    else
        alljoyn_execute_action(getp_action);

    Property* status_property = get_plug_vector_property_status(pos);
    Property* voltage_property = get_plug_vector_property_voltage(pos);
    Property* current_property = get_plug_vector_property_current(pos);
    Property* frequency_property = get_plug_vector_property_frequency(pos);
    Property* power_property = get_plug_vector_property_power(pos);
    Property* energy_property = get_plug_vector_property_energy(pos);

    if(status_property==NULL){
        cout << "Status not found" << endl << flush; 
    }else{
        const char* status = status_property->getPropertyValue().charValue;\
        if(strcmp(status, "Switch On")==0){
            muzzley_publish_plug_state(component, true);
        }else
            muzzley_publish_plug_state(component, false);
    }
    if(voltage_property==NULL){
        cout << "Voltage not found" << endl << flush; 
    }else{
        const char* voltage = voltage_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_VOLTAGE, voltage);
    }
    if(current_property==NULL){
        cout << "Current not found" << endl << flush; 
    }else{
        const char* current = current_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_CURRENT, current);
    }
    if(frequency_property==NULL){
        cout << "Frequency not found" << endl << flush; 
    }else{
        const char* freq = frequency_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_FREQUENCY, freq);
    }
    if(power_property==NULL){
        cout << "Power not found" << endl << flush; 
    }else{
        const char* power = power_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_POWER, power);
    }
    if(energy_property==NULL){
        cout << "Energy not found" << endl << flush; 
    }else{
        const char* energy = energy_property->getPropertyValue().charValue;
        muzzley_publish_plug_string(component, PROPERTY_ENERGY, energy);
    }
    muzzley_clean_request_vector();
}

bool muzzley_handle_plug_request(muzzley::JSONObjT _data){
    string io = (string)_data["d"]["p"]["io"];
    string component =  (string)_data["d"]["p"]["component"];
    string property = (string)_data["d"]["p"]["property"];
    string user_name = (string)_data["d"]["u"]["name"];
    string user_id = (string)_data["d"]["u"]["profileId"];
    bool isOnBehalfOf = (bool)_data["d"]["u"]["isOnBehalfOf"];
    string cid = (string)_data["h"]["cid"];
    int t = (int)_data["h"]["t"];
    
    if(!muzzley_OnBehalfOf){
        if(isOnBehalfOf){
            cout << "Ignoring request on behalf of user id: " << user_id << " Name: " << user_name << endl << flush;
            return false;
        }
    }

    if(!muzzley_plug_vector_check(component)){
        cout << "Received request for unknown plug id: " << component << " from iser id: " << user_id << " Name: " << user_name << endl << flush;
        return false;
    }

    print_request_vector();
    muzzley_plug_vector_print();
    
    int pos = get_plug_vector_pos(component);
    if (pos==-1)
        return false;

    if (io=="r"){
        cout << "Receiving read request for plug" << endl << flush;
        
        if(property==PROPERTY_STATUS){
            muzzley_handle_plug_read_status_request(component, cid, t);
        }
        if(property==PROPERTY_VOLTAGE){
            muzzley_handle_plug_read_voltage_request(component, cid, t);
        }
        if(property==PROPERTY_CURRENT){
            muzzley_handle_plug_read_current_request(component, cid, t);
        }
        if(property==PROPERTY_FREQUENCY){
            muzzley_handle_plug_read_frequency_request(component, cid, t);
        }
        if(property==PROPERTY_POWER){
            muzzley_handle_plug_read_power_request(component, cid, t);
        }
        if(property==PROPERTY_ENERGY){
            muzzley_handle_plug_read_energy_request(component, cid, t);
        }
    }
    if (io=="w"){
        if(property==PROPERTY_STATUS){
            bool bool_status = (bool)_data["d"]["p"]["data"]["value"];
            muzzley_handle_plug_write_status_request(component, bool_status);
            muzzley_update_plug_properties(component);
        }
    }
    
    return true;
}


//LightingSDK
class ControllerClientCallbackHandler : public ControllerClientCallback {
  public:

    ~ControllerClientCallbackHandler() { }

    void ConnectedToControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        muzzley_controllerservice_id=uniqueId;
        muzzley_controllerservice_name=name;
        muzzley_controllerservice_connected=true;
        printf("\n%s:\ncontrollerServiceDeviceID: %s\ncontrollerServiceName: %s\n\n", __func__, uniqueId.data(), name.data());
        lsf_controller_service_print();
    }

    void ConnectToControllerServiceFailedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        muzzley_controllerservice_id="";
        muzzley_controllerservice_name="";
        muzzley_controllerservice_connected=false;
        printf("\n%s:\ncontrollerServiceDeviceID: %s\ncontrollerServiceName: %s\n\n", __func__, uniqueId.data(), name.data());
        lsf_controller_service_print();
    }

    void DisconnectedFromControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        muzzley_controllerservice_id="";
        muzzley_controllerservice_name="";
        muzzley_controllerservice_connected=false;
        printf("\n%s:\ncontrollerServiceDeviceID: %s\ncontrollerServiceName: %s\n\n", __func__, uniqueId.data(), name.data());
        lsf_controller_service_print();
    }

    void ControllerClientErrorCB(const ErrorCodeList& errorCodeList) {
        printf("\n%s:", __func__);
        ErrorCodeList::const_iterator it = errorCodeList.begin();
        for (; it != errorCodeList.end(); ++it) {
            printf("\n%s", ControllerClientErrorText(*it));
        }
        printf("\n");
        lsf_controller_service_print();
    }
    

};

class ControllerServiceManagerCallbackHandler : public ControllerServiceManagerCallback {

    void ControllerServiceLightingResetCB(void) {
        printf("\n%s\n", __func__);
    }
    
    void ControllerServiceNameChangedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        muzzley_controllerservice_id=controllerServiceDeviceID;
        muzzley_controllerservice_name=controllerServiceName;
        printf("\n%s:\ncontrollerServiceDeviceID: %s\ncontrollerServiceName: %s\n\n", __func__, uniqueId.data(), name.data());
        lsf_controller_service_print();
    }


};

class LampManagerCallbackHandler : public LampManagerCallback {
private:
    muzzley::Client* _client;

public:
    LampManagerCallbackHandler(muzzley::Client* _c) : LampManagerCallback() {
        this->_client = _c;
    }

    void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampIDs) {
        printf("\n%s():\nresponseCode: %s\nlistsize: %lu", __func__, LSFResponseCodeText(responseCode), lampIDs.size());
        if (responseCode == LSF_OK) {
            //muzzley_lamplist.clear();
            LSFStringList::const_iterator it = lampIDs.begin();
            uint8_t count = 1;
            for (; it != lampIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).data());
                count++;
                muzzley_lamplist[(*it).data()] = muzzley_lamplist_get_lampname((*it).data());
            }
            printf("\n\n");
            lampList.clear();
            lampList = lampIDs;
            muzzley_replace_lighting_components();
        }
    }

    void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& lampName) {
        LSFString uniqueId = lampID;
        printf("\n%s:\nresponseCode; %s\nlampID: %s\nlanguage: %s\n", __func__, LSFResponseCodeText(responseCode), uniqueId.data(), language.data());
        if (responseCode == LSF_OK) {
            printf("lampName = %s\n\n", lampName.data());
            for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
                for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
                    if(local_it->first == lampID.data() ){
                        local_it->second = lampName.data();
                        cout << endl << "Lamp ID: " << lampID << endl << "Name: " << lampName << " was updated in lamplist" << endl << endl << flush; 
                      }
                }      
            }
            muzzley_add_lighting_component(lampID, lampName);
        }
    }

    void LampNameChangedCB(const LSFString& lampID, const LSFString& lampName) {
        printf("\n%s:\nlampID = %s\nlampName = %s", __func__, lampID.data(), lampName.data());
        muzzley_lamplist[lampID.data()]=lampName;
        muzzley_add_lighting_component(lampID, lampName);
    }
    
    void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampState& lampState) {
        LSFString uniqueId = lampID;
        printf("\n%s:\nresponseCode: %s\nlampID: %s", __func__, LSFResponseCodeText(responseCode), uniqueId.data());
        if (responseCode == LSF_OK) {
            printf("\nstate: %s\n", lampState.c_str());
            muzzley_parseLampState(lampID, lampState, this->_client);
        }
    }

    void LampStateChangedCB(const LSFString& lampID, const LampState& lampState) {
        printf("\n%s:\nlampID: %s\nlampState: \n\n%s\n\n", __func__, lampID.data(), lampState.c_str());
        muzzley_parseLampState(lampID, lampState, this->_client);
    }

    void LampsFoundCB(const LSFStringList& lampIDs) {
        printf("\n%s():\nlistsize:%lu", __func__, lampIDs.size());
        LSFStringList::const_iterator it = lampIDs.begin();
        uint8_t count = 1;
        for (; it != lampIDs.end(); ++it) {
            printf("\n(%d)%s\n", count, (*it).data());
            count++;
            muzzley_lamplist[(*it).data()]=muzzley_lamplist_get_lampname((*it).data());
        }
        printf("\n");
        muzzley_add_lighting_components(lampIDs);
        lampList.clear();
        lampList = lampIDs;
    }

    void LampsLostCB(const LSFStringList& lampIDs) {
        printf("\n%s():\nlistsize=%lu", __func__, lampIDs.size());
        LSFStringList::const_iterator it = lampIDs.begin();
        uint8_t count = 1;
        for (; it != lampIDs.end(); ++it) {
            printf("\n(%d)%s\n", count, (*it).data());
            muzzley_lamplist_del_lamp((*it).data());
            count++;
        }
        muzzley_remove_lighting_components(lampIDs);
        lampList.clear();
        lampList = lampIDs;
    }

};


class ConfigSession : public BusAttachment::JoinSessionAsyncCB, public SessionListener {
  public:
    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        if (status != ER_OK) {
            cout << "Error joining session " <<  QCC_StatusText(status) << endl;
            free(context);
        } else {
            ConfigClient configClient(*bus);
            int v = 0;
            bus->EnableConcurrentCallbacks();
            QStatus myStat = configClient.GetVersion((char*)context, v, sessionId);
            cout << "Status " << myStat << " returned when contacting config service, version=" << v << endl;
            free(context);
            bus->LeaveSession(sessionId);
            delete this;
        }
    }

};


class MyReceiver : public NotificationReceiver {
  public:
    virtual void Receive(Notification const& notification) {
        std::vector<NotificationText> vecMessages = notification.getText();

        for (std::vector<NotificationText>::const_iterator it = vecMessages.begin(); it != vecMessages.end(); ++it) {
            cout << "Notification in: " << it->getLanguage().c_str() << "  Message: " << it->getText().c_str() << endl;
        }

    }

    virtual void Dismiss(const int32_t msgId, const qcc::String appId) {
        cout << "Received notification dismiss for msg=" << msgId << " from app=" << appId.c_str() << endl;
    }
};

void muzzley_parse_plugs_controlpanelunit(string device_id_str, string device_name_str, ControlPanelControllerUnit* cp_unit){

    Property* plug_property_volt=NULL;
    Property* plug_property_curr=NULL;
    Property* plug_property_freq=NULL;
    Property* plug_property_watt=NULL;
    Property* plug_property_accu=NULL;
    Property* plug_property_status=NULL;
    Action* plug_action_on=NULL;
    Action* plug_action_off=NULL;
    Action* plug_action_get_properties=NULL;

    if(cp_unit==NULL)
        return;
    
    const qcc::String cp_unit_name = cp_unit->getUnitName(); 
    cout << "AnnounceHandler ControlPanelControllerUnit Name: " << cp_unit_name.c_str() << endl << flush;

    ControlPanel* cp_controlpanel = cp_unit->getControlPanel("rootContainer");
    if(cp_controlpanel==NULL){
        cout << "AnnounceHandler RootContainer not found!" << endl << flush;
    }else{
            qcc::String cp_panelname = cp_controlpanel->getPanelName();    
            cout << "AnnounceHandler ControlPanel Name: " << cp_panelname.c_str() << endl << flush;

            const qcc::String cd_path = cp_controlpanel->getObjectPath();
            cout << "AnnounceHandler ControlPanel Path: " << cd_path.c_str() << endl << flush;

            //Needed by some reason...
            sleep(1);

            Container* rootContainer = cp_controlpanel->getRootWidget("en");
            if (rootContainer==NULL) {
                cout << "AnnounceHandler RootContainer not found!" << endl << flush;
            }else{
                cout << "AnnounceHandler RootContainer found!" << endl << flush;
                if (rootContainer->getWidgetType() == 0) {
                    std::vector<Widget*> childWidgets = rootContainer->getChildWidgets();
                    cout << "AnnounceHandler Print ChildWidgets from rootContainer" << endl << endl << flush;
                    for (size_t i = 0; i < childWidgets.size(); i++) {
                        WidgetType widgetType = childWidgets[i]->getWidgetType();
                        qcc::String name = childWidgets[i]->getWidgetName();
                         bool secured = childWidgets[i]->getIsSecured();
                        bool enabled = childWidgets[i]->getIsEnabled();
                        bool writable = childWidgets[i]->getIsWritable();

                        uint32_t states = childWidgets[i]->getStates();
                        uint32_t bgcolor = childWidgets[i]->getBgColor();
                        const qcc::String label = childWidgets[i]->getLabel();
                        cout << "Widget Type: " << widgetType << endl << flush;
                        std::vector<Widget*> childchildWidgets;
                            
                            switch(widgetType){
                                case WIDGET_TYPE_ACTION:
                                    cout << "Widget Type: ACTION" << endl << flush;
                                    break;
                                case WIDGET_TYPE_ACTION_WITH_DIALOG:
                                    cout << "Widget Type: ACTION_WITH_DIALOG" << endl << flush;
                                    break;
                                case WIDGET_TYPE_LABEL:
                                    cout << "Widget Type: LABEL" << endl << flush;
                                    break;
                                case WIDGET_TYPE_PROPERTY:
                                    cout << "Widget Type: PROPERTY" << endl << flush;
                                    plug_property_status=((Property*)childWidgets[i]);
                                    break;
                                case WIDGET_TYPE_CONTAINER:
                                    cout << "Widget Type: CONTAINER" << endl << endl << flush;
                             
                                    childchildWidgets = ((Container*)childWidgets[i])->getChildWidgets();
                                    for (size_t j = 0; j < childchildWidgets.size(); j++) {
                                        widgetType = childchildWidgets[j]->getWidgetType();
                                        name = childchildWidgets[j]->getWidgetName();
                                        secured = childchildWidgets[j]->getIsSecured();
                                        enabled = childchildWidgets[j]->getIsEnabled();
                                        writable = childchildWidgets[j]->getIsWritable();

                                        states = childchildWidgets[j]->getStates();
                                        bgcolor = childchildWidgets[j]->getBgColor();
                                        const qcc::String label = childchildWidgets[j]->getLabel();
                                        cout << "    Widget Type: " << widgetType << endl << flush;
                                            switch(widgetType){
                                                case WIDGET_TYPE_ACTION:
                                                    cout << "    Widget Type: ACTION" << endl << flush;
                                                    if(label=="On")
                                                        plug_action_on=((Action*)childchildWidgets[j]);
                                                    else if (label=="Off")
                                                        plug_action_off=((Action*)childchildWidgets[j]);
                                                    else if (label=="Get Properties")
                                                        plug_action_get_properties=((Action*)childchildWidgets[j]);
                                                    break;
                                                case WIDGET_TYPE_ACTION_WITH_DIALOG:
                                                    cout << "    Widget Type: ACTION_WITH_DIALOG" << endl << flush;
                                                    break;
                                                case WIDGET_TYPE_LABEL:
                                                    cout << "    Widget Type: LABEL" << endl << flush;
                                                    break;
                                                case WIDGET_TYPE_PROPERTY:
                                                    cout << "    Widget Type: PROPERTY" << endl << flush;
                                                    if(label=="Volt(V):"){
                                                        plug_property_volt=((Property*)childchildWidgets[j]);
                                                        plug_property_volt->setValue("");
                                                    }
                                                    if(label=="Curr(A):"){
                                                        plug_property_curr=((Property*)childchildWidgets[j]);
                                                        plug_property_curr->setValue("");
                                                    }
                                                    if(label=="Freq(Hz):"){
                                                        plug_property_freq=((Property*)childchildWidgets[j]);
                                                        plug_property_freq->setValue("");
                                                    }
                                                    if(label=="Watt(W):"){
                                                        plug_property_watt=((Property*)childchildWidgets[j]);
                                                        plug_property_watt->setValue("");
                                                    }
                                                    if(label=="ACCU(KWH):"){
                                                        plug_property_accu=((Property*)childchildWidgets[j]);
                                                        plug_property_accu->setValue("");
                                                    }
                                                    break;
                                                case WIDGET_TYPE_CONTAINER:
                                                    cout << "    Widget Type: CONTAINER" << endl << flush;
                                                    break;
                                                case WIDGET_TYPE_DIALOG:
                                                    cout << "    Widget Type: DIALOG" << endl << flush;
                                                    break;
                                                case WIDGET_TYPE_ERROR:
                                                    cout << "    Widget Type: ERROR" << endl << flush;
                                                    break;
                                                
                                            }
                                            cout << "    Label: " << label << endl << flush;
                                            cout << "    Secured: " << secured << endl << flush;
                                            cout << "    Enabled: " << enabled << endl << flush;
                                            cout << "    Writable: " << writable << endl << flush;
                                            cout << "    States: " << states << endl << flush;
                                            cout << "    BGcolor: " << bgcolor << endl << endl << flush;
                                        }
                                        

                                    break;
                                case WIDGET_TYPE_DIALOG:
                                    cout << "Widget Type: DIALOG"<< endl << flush;
                                    break;
                                case WIDGET_TYPE_ERROR:
                                    cout << "Widget Type: ERROR"<< endl << flush;
                                    break;
                                
                            }
                            cout << "Label: " << label << endl << flush;
                            cout << "Secured: " << secured << endl << flush;
                            cout << "Enabled: " << enabled << endl << flush;
                            cout << "Writable: " << writable << endl << flush;
                            cout << "States: " << states << endl << flush;
                            cout << "BGcolor: " << bgcolor << endl << endl << flush;
                            
                    }
                    
                }

                else if (rootContainer->getWidgetType() == 1) {
                    cout << "AnnounceHandler Widget type->DIALOG" << endl;
                } else {
                    std::cout << "AnnounceHandler RootWidget is of unknown type" << std::endl;
                }
                

            }
        }

    del_plug_vector_pos(device_id_str);
    add_plug_vector_pos(device_id_str, device_name_str, plug_property_status, plug_property_volt, plug_property_curr, plug_property_freq, plug_property_watt , plug_property_accu, plug_action_get_properties, plug_action_on, plug_action_off);
    muzzley_add_plugs_component(device_id_str, device_name_str);
    gupnp_generate_plugs_XML();
    muzzley_plug_vector_print();
}

static void announceHandlerCallback(qcc::String const& busName, unsigned short version, unsigned short port, const AnnounceHandler::ObjectDescriptions& objectDescs, const AnnounceHandler::AboutData& aboutData){
    try{

        const char* app_name;
        const char* default_language;
        const char* device_id;
        const char* device_name;
        const char* manufacturer;
        const char* model_number;

        string app_name_str;
        string default_language_str;
        string device_id_str;
        string device_name_str;
        string manufacturer_str;
        string model_number_str;

        for (AboutClient::AboutData::const_iterator it = aboutData.begin(); it != aboutData.end(); ++it) {
            qcc::String key = it->first;
            ajn::MsgArg value = it->second;

            if(strcmp(key.c_str(),"AppName")==0){
                app_name=value.v_string.str;
                app_name_str = string(app_name);
            }
            if(strcmp(key.c_str(),"DeviceId")==0){
                device_id=value.v_string.str;
                device_id_str = string(device_id);
            }
            if(strcmp(key.c_str(),"DefaultLanguage")==0){
                default_language=value.v_string.str;
                default_language_str = string(default_language);
            }
            if(strcmp(key.c_str(),"DeviceName")==0){
                device_name=value.v_string.str;
                device_name_str = string(device_name);
            }
            if(strcmp(key.c_str(),"Manufacturer")==0){
                manufacturer=value.v_string.str;
                manufacturer_str = string(manufacturer);
            }
            if(strcmp(key.c_str(),"ModelNumber")==0){
                model_number=value.v_string.str;
                model_number_str = string(model_number);
            }

        }

        cout << endl << "AnnounceHandler AboutData:" << endl << flush;
        cout << "AppName: " << app_name_str << endl << flush;
        cout << "DeviceID: " << device_id_str << endl << flush;
        cout << "DeviceName: " << device_name_str << endl << flush;
        cout << "Manufacturer: " << manufacturer_str << endl << flush;
        cout << "ModelNumber: " << model_number_str << endl << endl << flush;
    
        cout << "AnnounceHandler ObjectDescriptions:" << endl;
        for (AboutClient::ObjectDescriptions::const_iterator it = objectDescs.begin(); it != objectDescs.end(); ++it) {
            qcc::String key = it->first;
            std::vector<qcc::String> vector = it->second;

            if(key=="/org/allseen/LSF/Lamp"){
                gupnp_generate_lighting_XML();
                muzzley_lamplist_update_lampname(device_id_str, device_name_str);
                muzzley_add_lighting_component(device_id_str, device_name_str);
            }

            if(key=="/org/allseen/LSF/ControllerService"){}
            if(key=="/ControlPanel/SmartPlug/rootContainer"){}

        }
 
        ControlPanelDevice* device = controlPanelController->createControllableDevice(busName, objectDescs);
        
        if(device==NULL){
            cout << "AnnounceHandler ControlPanelDevice not found" << endl << flush;
            return;
        }

        std::cout << "AnnounceHandler session established with device: " << device->getDeviceBusName().c_str() << std::endl;
        std::map<qcc::String, ControlPanelControllerUnit*> units = device->getDeviceUnits();
        std::map<qcc::String, ControlPanelControllerUnit*>::iterator iter;
        std::map<qcc::String, ControlPanel*>::iterator it;

        for (iter = units.begin(); iter != units.end(); iter++) {
            std::cout << endl << "AnnounceHandler parsing unit: " << iter->first.c_str() << std::endl;
            
            std::map<qcc::String, ControlPanel*> controlPanels = iter->second->getControlPanels();
            for (it = controlPanels.begin(); it != controlPanels.end(); it++) {
                std::cout << "AnnounceHandler parsing panelName: " << it->first.c_str() << std::endl;
                
                std::vector<qcc::String> languages = it->second->getLanguageSet().getLanguages();
                for (size_t i = 0; i < languages.size(); i++) {
                    std::cout << "AnnounceHandler parsing language: " << languages[i].c_str() << std::endl;
                    Container* rootContainer = it->second->getRootWidget(languages[i]);
                    std::cout << "AnnounceHandler finished loading widget: " << rootContainer->getWidgetName().c_str() << std::endl;
                }
            }
        }
        

        QStatus status = device->startSessionAsync();
        cout << "AnnounceHandler Session Status: " <<  QCC_StatusText(status) << endl << flush;

        const qcc::String& cd_busname = device->getDeviceBusName();
           cout << "AnnounceHandler ControlPanel Device BusName: " << cd_busname << endl << flush;
           //const ajn::SessionId cd_sessionid = device->getSessionId();
           //cout << "AnnounceHandler ControlPanel Device SessionId: " << cd_sessionid << endl << flush;
           
        //org/allseen/LSF/Lamp
        //ControlPanel/SmartPlug/rootContainer
        ControlPanelControllerUnit* cp_unit = device->getControlPanelUnit("ControlPanel/SmartPlug/rootContainer");
        if(cp_unit==NULL){
            cout << "AnnounceHandler SmartPlug not found" << endl << flush;
            return;
        }

        //ControlPanelSessionHandler cp_sessionHandler = ControlPanelSessionHandler(device);
        //ajn::SessionId sessionid = cp_sessionHandler.getSessionId();

        //ProxyBusObject pbus_obj = ProxyBusObject(**bus, "SmartPlug", "ControlPanel/SmartPlug/rootContainer", sessionid, false);
        //IntrospectRemoteObjectAsync();
        
        muzzley_parse_plugs_controlpanelunit(device_id_str, device_name_str, cp_unit);

    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
    }
    
}

void cleanup() {
    if (!bus) {
        return;
    }
    
    if (announceHandler) {
        AnnouncementRegistrar::UnRegisterAnnounceHandler(*bus, *announceHandler, NULL, 0);
        delete announceHandler;
    }

    if (notificationService) {
        notificationService->shutdown();
        notificationService = 0;
    }
    
    bus->Disconnect(); 
    bus->Stop();
    delete bus;
    bus = 0;
}


void upnp_info(string urn, string host, string xml_file_path, int port,
            string interface, string friendlyname,string udn, string serialnumber,
            string manufacturer, string manufacturer_url, string modelname,
            string modelnumber, string modeldescription){

    cout << "URN: " << urn << endl << flush;
    cout << "HOST: " << host << endl << flush;
    cout << "XML FILE PATH: " << xml_file_path << endl << flush;
    cout << "PORT: " << port << endl << flush;
    cout << "INTERFACE: " << interface << endl << flush;
    cout << "FRIENDLY NAME: " << friendlyname << endl << flush;
    cout << "UDN: " << udn << endl << flush;
    cout << "SERIAL NUMBER: " << serialnumber << endl << flush;
    cout << "MANUFACTURER: " << manufacturer << endl << flush;
    cout << "MANUFACTURER URL: " << manufacturer_url << endl << flush;
    cout << "MODEL NAME: " << modelname << endl << flush;
    cout << "MODEL NUMBER: " << modelnumber << endl << flush;
    cout << "MODEL DESCRIPTION: " << modeldescription << endl << endl << flush;
}

void upnp_advertise(){

    GMainLoop *main_loop; 
    GError *error = NULL;
    GUPnPContext *gupnp_lighting_context;
    GUPnPRootDevice *gupnp_lighting_dev;
    GSSDPResourceGroup *gupnp_lighting_resource_group;
    
    GUPnPContext *gupnp_plugs_context;
    GUPnPRootDevice *gupnp_plugs_dev;
    GSSDPResourceGroup *gupnp_plugs_resource_group;


    #if !GLIB_CHECK_VERSION(2,35,0)
       g_type_init ();
    #endif
  
    //Lighting UPnP
    gupnp_lighting_context = gupnp_context_new (NULL, muzzley_lighting_upnp_interface.c_str(), muzzley_lighting_upnp_port, &error);
    if (error) {
        g_printerr ("Error creating the GUPnP lighting context: %s\n", error->message);
        g_error_free (error);
        return EXIT_FAILURE;
    }
    
    gupnp_generate_lighting_XML();

    gupnp_lighting_dev = gupnp_root_device_new (gupnp_lighting_context, MUZZLEY_LIGHTING_XML_FILENAME, ".");
    gupnp_root_device_set_available (gupnp_lighting_dev, TRUE);
    gupnp_lighting_resource_group = gupnp_root_device_get_ssdp_resource_group(gupnp_lighting_dev);
    gssdp_resource_group_set_max_age (gupnp_lighting_resource_group, GUPNP_MAX_AGE);
    gssdp_resource_group_set_message_delay(gupnp_lighting_resource_group, GUPNP_MESSAGE_DELAY);

    std::stringstream lighting_device_urn;
    lighting_device_urn << "urn:Muzzley:device:" << muzzley_lighting_profileid << ":1";
    muzzley_lighting_upnp_urn = lighting_device_urn.str();
    
    std::stringstream lighting_host;
    lighting_host << "http://" << gupnp_context_get_host_ip (gupnp_lighting_context) << ":" << gupnp_context_get_port (gupnp_lighting_context) << "/" << gupnp_root_device_get_relative_location(gupnp_lighting_dev);
    muzzley_lighting_upnp_host = lighting_host.str(); 
    
    muzzley_lighting_upnp_port=gupnp_context_get_port (gupnp_lighting_context);
    muzzley_lighting_upnp_xml_filepath=gupnp_root_device_get_description_path(gupnp_lighting_dev);

    //Plugs UPnP
    gupnp_plugs_context = gupnp_context_new (NULL, muzzley_plugs_upnp_interface.c_str(), muzzley_plugs_upnp_port, &error);
    if (error) {
        g_printerr ("Error creating the GUPnP plugs context: %s\n", error->message);
        g_error_free (error);
        return EXIT_FAILURE;
    }

    gupnp_generate_plugs_XML();

    gupnp_plugs_dev = gupnp_root_device_new (gupnp_plugs_context, MUZZLEY_PLUGS_XML_FILENAME, ".");
    gupnp_root_device_set_available (gupnp_plugs_dev, TRUE);
    gupnp_plugs_resource_group = gupnp_root_device_get_ssdp_resource_group(gupnp_plugs_dev);
    gssdp_resource_group_set_max_age (gupnp_plugs_resource_group, GUPNP_MAX_AGE);
    gssdp_resource_group_set_message_delay(gupnp_plugs_resource_group, GUPNP_MESSAGE_DELAY);

    std::stringstream plugs_device_urn;
    plugs_device_urn << "urn:Muzzley:device:" << muzzley_plugs_profileid << ":1";
    muzzley_plugs_upnp_urn = plugs_device_urn.str();   
   
    std::stringstream plugs_host;
    plugs_host << "http://" << gupnp_context_get_host_ip (gupnp_plugs_context) << ":" << gupnp_context_get_port (gupnp_plugs_context) << "/" << gupnp_root_device_get_relative_location(gupnp_plugs_dev);
    muzzley_plugs_upnp_host = plugs_host.str(); 

    muzzley_plugs_upnp_port=gupnp_context_get_port (gupnp_plugs_context);
    muzzley_plugs_upnp_xml_filepath=gupnp_root_device_get_description_path(gupnp_plugs_dev);

    //gssdp_resource_group_add_resource_simple(gupnp_lighting_resource_group, lighting_device_urn_char, lighting_device_urn_char , lighting_host_char );
    //gssdp_resource_group_add_resource_simple(gupnp_plugs_resource_group, plugs_device_urn_char, plugs_device_urn_char , plugs_host_char );
    //gssdp_resource_group_set_available (gupnp_lighting_resource_group, TRUE);
    //gssdp_resource_group_set_available (gupnp_plugs_resource_group, TRUE);

    //Run the main loop
    main_loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (main_loop);
}


void upnp_info_print(){
    cout << endl << "GUPNP LIGHTING INFO:" << endl << flush;
    upnp_info(muzzley_lighting_upnp_urn, muzzley_lighting_upnp_host, muzzley_lighting_upnp_xml_filepath, muzzley_lighting_upnp_port,
            muzzley_lighting_upnp_interface, muzzley_lighting_upnp_friendlyname, muzzley_lighting_upnp_udn, muzzley_lighting_upnp_serialnumber,
            muzzley_manufacturer, muzzley_manufacturer_url, muzzley_modelname, muzzley_modelnumber, muzzley_modeldescription);
    cout << endl << "GUPNP PLUGS INFO:" << endl << flush;
    upnp_info(muzzley_plugs_upnp_urn, muzzley_plugs_upnp_host, muzzley_plugs_upnp_xml_filepath, muzzley_plugs_upnp_port,
            muzzley_plugs_upnp_interface, muzzley_plugs_upnp_friendlyname, muzzley_plugs_upnp_udn, muzzley_plugs_upnp_serialnumber,
            muzzley_manufacturer, muzzley_manufacturer_url, muzzley_modelname, muzzley_modelnumber, muzzley_modeldescription);
}

void cmd_line_parser_help(){
    cout << "Alljoyn-Muzzley Connector CMD line help:" << endl << flush;
    cout << "--core-endpointhost            set the Muzzley Core Endpointhost" << endl << flush;
    cout << "--api-endpointhost             set the Muzzley API Endpointhost" << endl << flush;
    cout << "--api-port                     set the Muzzley API Port number" << endl << flush;
    cout << "--manager-endpointhost         set the Muzzley Manager Endpointhost" << endl << flush;
    cout << "--manager-port                 set the Muzzley Manager Port number" << endl << flush;
    cout << "--lighting-profileid           set the Muzzley Lighting Profileid" << endl << flush;
    cout << "--lighting-app-token           set the Muzzley Lighting AppToken" << endl << flush;
    cout << "--lighting-friendlyname        set the Muzzley Lighting UPNP Friendlyname" << endl << flush;
    cout << "--lighting-interface           set the Muzzley Lighting UPNP Interface" << endl << flush;
    cout << "--lighting-port                set the Muzzley Lighting UPNP Port" << endl << flush;
    cout << "--plugs-profileid              set the Muzzley Plugs Profileid" << endl << flush;
    cout << "--plugs-app-token              set the Muzzley Plugs AppToken" << endl << flush; 
    cout << "--plugs-friendlyname           set the Muzzley Plugs UPNP Friendlyname" << endl << flush;       
    cout << "--plugs-interface              set the Muzzley Plugs UPNP Interface" << endl << flush;
    cout << "--plugs-port                   set the Muzzley Plugs UPNP Port" << endl << flush;
    cout << "--color-mode                   set the Muzzley Interface Color Mode" << endl << flush;
    cout << "--manufacturer                 set the UPnP Manufacturer string" << endl << flush;
    cout << "--manufacturer-url             set the UPnP Manufacturer URL" << endl << flush;
    cout << "--model-name                   set the UPnP Modelname string" << endl << flush;
    cout << "--model-number                 set the UPnP Model Number" << endl << flush;
    cout << "--model-description            set the UPnP Model Description string" << endl << flush;
    cout << "--ignore-onbehalfof            ignore requests on behalf of" << endl << flush;
    cout << "--help                         show this help text" << endl << endl << flush;
}

void muzzley_info_print(){
    cout << endl << "MUZZLEY INFO:" << endl << flush;
    cout << "CORE ENDPOINTHOST: " << muzzley_core_endpointhost << endl << flush;
    cout << "API ENDPOINTHOST: " << muzzley_api_endpointhost << endl << flush;
    cout << "API PORT: " << muzzley_api_port << endl << flush;
    cout << "MANAGER ENDPOINTHOST: " << muzzley_manager_endpointhost << endl << flush;
    cout << "MANAGER PORT: " << muzzley_manager_port << endl << flush;
    cout << "LIGHTING PROFILEID: " << muzzley_lighting_profileid << endl << flush;
    cout << "LIGHTING APPTOKEN: " << muzzley_lighting_apptoken << endl << flush;
    cout << "LIGHTING CHANNEL ID: " << muzzley_lighting_deviceKey << endl << flush;
    cout << "LIGHTING SESSION ID: " << muzzley_lighting_sessionid << endl << flush;
    cout << "LIGHTING FRIENDLYNAME: " << muzzley_lighting_upnp_friendlyname << endl << flush;
    cout << "PLUGS PROFILEID: " << muzzley_plugs_profileid << endl << flush;
    cout << "PLUGS APPTOKEN: " << muzzley_plugs_apptoken << endl << flush;
    cout << "PLUGS CHANNEL ID: " << muzzley_plugs_deviceKey << endl << flush;
    cout << "PLUGS SESSION ID: " << muzzley_plugs_sessionid<< endl << flush;
    cout << "PLUGS FRIENDLYNAME: " << muzzley_plugs_upnp_friendlyname << endl << flush;
    cout << "COLOR MODE: " << muzzley_color_mode << endl << flush;
    cout << "WORKERS: " << boolalpha << muzzley_OnBehalfOf << endl << endl << flush;
}

void alljoyn_status_info(){
    while(true){
        muzzley_info_print();
        upnp_info_print();
        lsf_controller_service_print();
        muzzley_lamplist_print();
        muzzley_plug_vector_print();
        sleep(MUZZLEY_DEFAULT_STATUS_INTERVAL);
    }
}

void muzzley_read_request_cleaner(){
    while(true){
        muzzley_clean_request_vector();
    }
}

int main(int argc, char* argv[]){
    
    muzzley_write_semaphore_file();

    // Adds listeners for SIGKILL, for gracefull stop
    // It will be invoked when the user hits Ctrl-c
    // For a list of allocated sempahores (and other shared resources), execute the 'ipcs' command on a tty
    struct sigaction action;
    action.sa_handler = semaphore_stop;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTERM, &action, 0);
    sigaction(SIGQUIT, &action, 0);
    sigaction(SIGINT, &action, 0);

    //Muzzley Client
    muzzley::Client _muzzley_lighting_client;

    //Set default Muzzley tokens
    muzzley_color_mode=PROPERTY_COLOR_HSV;
    muzzley_core_endpointhost=MUZZLEY_DEFAULT_CORE_ENDPOINTHOST;
    muzzley_api_endpointhost=MUZZLEY_DEFAULT_API_ENDPOINTHOST;
    muzzley_api_port=MUZZLEY_DEFAULT_API_PORT;
    muzzley_manager_endpointhost=MUZZLEY_DEFAULT_MANAGER_ENDPOINTHOST;
    muzzley_manager_port=MUZZLEY_DEFAULT_MANAGER_PORT;
    muzzley_manager_register_url=MUZZLEY_DEFAULT_MANAGER_REGISTER_URL;
    muzzley_manager_components_url=MUZZLEY_DEFAULT_MANAGER_COMPONENTS_URL;
    
    muzzley_lighting_profileid=MUZZLEY_DEFAULT_LIGHTING_PROFILEID;
    muzzley_lighting_apptoken=MUZZLEY_DEFAULT_LIGHTING_APP_TOKEN;
    muzzley_lighting_upnp_friendlyname=MUZZLEY_DEFAULT_LIGHTING_FRIENDLYNAME;
    muzzley_lighting_upnp_udn=MUZZLEY_DEFAULT_LIGHTING_UDN;
    muzzley_lighting_upnp_serialnumber=MUZZLEY_DEFAULT_LIGHTING_SERIALNUMBER;
    muzzley_lighting_upnp_interface = MUZZLEY_DEFAULT_NETWORK_INTERFACE;
    muzzley_lighting_upnp_port = MUZZLEY_DEFAULT_NETWORK_LIGHTING_PORT;

    muzzley_plugs_profileid=MUZZLEY_DEFAULT_PLUGS_PROFILEID;
    muzzley_plugs_apptoken=MUZZLEY_DEFAULT_PLUGS_APP_TOKEN;
    muzzley_plugs_upnp_friendlyname=MUZZLEY_DEFAULT_PLUGS_FRIENDLYNAME;
    muzzley_plugs_upnp_udn=MUZZLEY_DEFAULT_PLUGS_UDN;
    muzzley_plugs_upnp_serialnumber=MUZZLEY_DEFAULT_PLUGS_SERIALNUMBER;
    muzzley_plugs_upnp_interface = MUZZLEY_DEFAULT_NETWORK_INTERFACE;
    muzzley_plugs_upnp_port = MUZZLEY_DEFAULT_NETWORK_PLUGS_PORT;

    muzzley_manufacturer=MUZZLEY_DEFAULT_MANUFACTURER;
    muzzley_manufacturer_url=MUZZLEY_DEFAULT_MANUFACTURER_URL;
    muzzley_modelname=MUZZLEY_DEFAULT_MODELNAME;
    muzzley_modelnumber=MUZZLEY_DEFAULT_MODELNUMBER;
    muzzley_modeldescription=MUZZLEY_DEFAULT_MODELDESCRIPTION;

    muzzley_OnBehalfOf = true;


    //Get MACAdress info from the current network interface in use
    muzzley_lighting_macAddress = get_iface_macAdress(muzzley_network_interface);
       muzzley_plugs_macAddress = get_iface_macAdress(muzzley_network_interface);

    //Parse cmd line custom Muzzley tokens
    if(argc>1){
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i],"--core-endpointhost")==0) {
                muzzley_core_endpointhost = argv[i + 1];
                _muzzley_lighting_client.setEndpointHost(muzzley_core_endpointhost);
                _muzzley_plugs_client.setEndpointHost(muzzley_core_endpointhost);
            } else if (strcmp(argv[i], "--api-endpointhost")==0) {
                muzzley_api_endpointhost = argv[i + 1];
            } else if (strcmp(argv[i], "--api-port")==0) {
                muzzley_api_port = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--manager-endpointhost")==0) {
                muzzley_manager_endpointhost = argv[i + 1];
            } else if (strcmp(argv[i], "--manager-port")==0) {
                muzzley_manager_port = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--lighting-profileid")==0) {
                muzzley_lighting_profileid = argv[i + 1];
            } else if (strcmp(argv[i], "--lighting-app-token")==0) {
                muzzley_lighting_apptoken = argv[i + 1];  
            } else if (strcmp(argv[i], "--lighting-friendlyname")==0) {
                muzzley_lighting_upnp_friendlyname = argv[i + 1];
            } else if (strcmp(argv[i], "--lighting-serialnumber")==0) {
                muzzley_lighting_upnp_serialnumber = argv[i + 1];
            } else if (strcmp(argv[i], "--lighting-udn")==0) {
                muzzley_lighting_upnp_udn = argv[i + 1];  
            } else if (strcmp(argv[i], "--lighting-interface")==0) {
                muzzley_lighting_upnp_interface = argv[i + 1];
            } else if (strcmp(argv[i], "--lighting-port")==0) {
                muzzley_lighting_upnp_port = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--plugs-profileid")==0) {
                muzzley_plugs_profileid = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-app-token")==0) {
                muzzley_plugs_apptoken = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-friendlyname")==0) {
                muzzley_plugs_upnp_friendlyname = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-serialnumber")==0) {
                muzzley_plugs_upnp_serialnumber = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-udn")==0) {
                muzzley_plugs_upnp_udn = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-interface")==0) {
                muzzley_lighting_upnp_interface = argv[i + 1];
            } else if (strcmp(argv[i], "--plugs-port")==0) {
                muzzley_plugs_upnp_port = atoi(argv[i + 1]);
            } else if (strcmp(argv[i], "--color-mode")==0) {
                if(strcmp(argv[i + 1], PROPERTY_COLOR_RGB)==0){
                	muzzley_color_mode=PROPERTY_COLOR_RGB;
                } else if (strcmp(argv[i + 1], PROPERTY_COLOR_HSV)==0){
                	muzzley_color_mode=PROPERTY_COLOR_HSV;
                } else if (strcmp(argv[i + 1], PROPERTY_COLOR_HSVT)==0){
                    muzzley_color_mode=PROPERTY_COLOR_HSVT;
                } else {
                	cout << "Unknown color mode. Using HSV as default." << endl << flush;
                	muzzley_color_mode=PROPERTY_COLOR_HSV;
                }
            } else if (strcmp(argv[i], "--manufacturer")==0) {
                muzzley_manufacturer = argv[i + 1];
            } else if (strcmp(argv[i], "--manufacturer-url")==0) {
                muzzley_manufacturer_url = argv[i + 1];
            } else if (strcmp(argv[i], "--model-name")==0) {
                muzzley_modelname = argv[i + 1];
            } else if (strcmp(argv[i], "--model-number")==0) {
                muzzley_modelnumber = argv[i + 1];
            } else if (strcmp(argv[i], "--model-description")==0) {
                muzzley_modeldescription = argv[i + 1];
            } else if (strcmp(argv[i], "--ignore-onbehalfof")==0) {
                muzzley_OnBehalfOf = false;
            } else if (strcmp(argv[i], "--help")==0) {
                cmd_line_parser_help();
                exit(0);
            }else {
                cmd_line_parser_help();
                exit(0);
            }
            i++;
        }
    }
    
    //Initialize alljoyn bus ("ClientTest")
    bus = new BusAttachment("MuzzleyConnector", true);
    QStatus bus_status = bus->Start();
    if (ER_OK != bus_status) {
        cout << "Error starting bus: " << QCC_StatusText(bus_status) << endl;
        cleanup();
        return 1;
    }

    bus_status = bus->Connect();
    if (ER_OK != bus_status) {
        cout << "Error connecting bus: " << QCC_StatusText(bus_status) << endl;
        cleanup();
        return 1;
    }
  
    ControllerClientCallbackHandler controllerClientCBHandler;
    ControllerServiceManagerCallbackHandler controllerServiceManagerCBHandler;
    LampManagerCallbackHandler lampManagerCBHandler(&_muzzley_lighting_client);
    ControllerClient client(*bus, controllerClientCBHandler);
    ControllerServiceManager controllerServiceManager(client, controllerServiceManagerCBHandler);
    LampManager lampManager(client, lampManagerCBHandler);
   
    ControllerClientStatus status = client.Start();
    status = CONTROLLER_CLIENT_OK;
 
    printf("\nLighting Controller Client Start() returned %s\n", ControllerClientStatusText(status));
    printf("\nLighting Controller Client Version = %d\n", client.GetVersion());
  
    /*
    //Initialize notification consumer
    notificationService = NotificationService::getInstance();
    MyReceiver receiver;
    bus_status = notificationService->initReceive(bus, &receiver);
    if (ER_OK != bus_status) {
        cout << "Error initializing notification receiver: " << QCC_StatusText(bus_status) << endl;
        cleanup();
        return 1;
    }
    */

    //Register for controlpanel announcements
    controlPanelService = ControlPanelService::getInstance();
    QCC_SetDebugLevel(logModules::CONTROLPANEL_MODULE_LOG_NAME, logModules::ALL_LOG_LEVELS);
    
    controlPanelController = new ControlPanelController();
    controlPanelListener = new ControlPanelListenerImpl(controlPanelController);
    bus_status = controlPanelService->initController(bus, controlPanelController, controlPanelListener);
    if (bus_status != ER_OK) {
        std::cout << "Could not initialize Controllee." << std::endl;
        //cleanup();
        //return 1;
    }

    //Register for controlpanel notification announcements
    announceHandler = new AnnounceHandlerImpl(NULL, announceHandlerCallback);
    AnnouncementRegistrar::RegisterAnnounceHandler(*bus, *announceHandler, NULL, 0);
    conService = NotificationService::getInstance();
    controller_receiver = new ControllerNotificationReceiver(controlPanelController);
    bus_status = conService->initReceive(bus, controller_receiver);
    if (bus_status != ER_OK) {
        std::cout << "Could not initialize receiver." << std::endl;
        //cleanup();
        //return 1;
    }
    
    _muzzley_lighting_client.setLoopAssynchronous(MUZZLEY_LOOPASSYNCHRONOUS);
    _muzzley_plugs_client.setLoopAssynchronous(MUZZLEY_LOOPASSYNCHRONOUS);
    
    _muzzley_lighting_client.on(muzzley::AppLoggedIn,[&lampManager] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool{
        muzzley_lighting_sessionid = (string)_data["d"]["sessionId"];
        //cout << "Lighting logged in with session id: " << muzzley_lighting_sessionid << endl << endl << flush;

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_lighting_profileid);
        _s1.setChannel(muzzley_lighting_deviceKey);
        //_s1.setComponent("*");
        //_s1.setProperty("*");

        _muzzley_lighting_client.on(muzzley::Published, _s1, [&lampManager] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool {
            //_data->prettify(cout);
            //cout << endl << flush;
            cout << "Request: " << _data << endl << flush;

            try{
                muzzley::JSONObjT m = *_data;
                muzzley_handle_lighting_request(lampManager, m);
            }catch(exception& e){
                cout << "Exception: " << e.what() << endl << flush;
            }
                
            return true;
        });
            
        return true;
    });

    _muzzley_plugs_client.on(muzzley::AppLoggedIn,[] (muzzley::Message& _data, muzzley::Client& _muzzley_plugs_client) -> bool{
        muzzley_plugs_sessionid = (string)_data["d"]["sessionId"];
        //cout << "Plugs logged in with session id: " << muzzley_plugs_sessionid << endl << endl << flush;
       
        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(muzzley_plugs_profileid);
        _s1.setChannel(muzzley_plugs_deviceKey);
        //_s1.setComponent("*");
        //_s1.setProperty("*");

        _muzzley_plugs_client.on(muzzley::Published, _s1, [] (muzzley::Message& _data, muzzley::Client& _muzzley_plugs_client) -> bool {
            //_data->prettify(cout);
            //cout << endl << flush;
            cout << "Request:" << _data << endl << flush;

            try{
                muzzley::JSONObjT m = *_data;
                muzzley_handle_plug_request(m);
            }catch(exception& e){
                cout << "Exception: " << e.what() << endl << flush;
            }
                
            return true;
        });
        return true;
    });
    
    
    _muzzley_lighting_client.on(muzzley::Reconnect, [] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool {
        cout << "Reconnecting Muzzley Lighting Client..." << endl << flush;
        return true;
    });

    _muzzley_plugs_client.on(muzzley::Reconnect, [] (muzzley::Message& _data, muzzley::Client& _muzzley_plugs_client) -> bool {
        cout << "Reconnecting Muzzley Plugs Client..." << endl << flush;
        return true;
    });
   

    try{
        // Waits for global manager lighting devicekey
        while(!muzzley_lighting_registered){
            cout << endl << "Waiting for Muzzley lighting registration..." << endl << flush;
            if(muzzley_lighting_connect_API()==true){
                muzzley_lighting_registered=muzzley_lighting_connect_manager();
                sleep(1);
            }
            
        }

        // Waits for global manager plugs devicekey
        while(!muzzley_plugs_registered){
            cout << endl << "Waiting for Muzzley plugs registration..." << endl << flush;
            if(muzzley_plugs_connect_API()==true){
                muzzley_plugs_registered=muzzley_plugs_connect_manager();
                sleep(1);
            }
        }

        //Connects the application to the Muzzley server.
        _muzzley_lighting_client.initApp(muzzley_lighting_apptoken);
        cout << "Muzzley lighting started!" << endl << flush;

        //Connects the application to the Muzzley server.
        _muzzley_plugs_client.initApp(muzzley_plugs_apptoken);
        cout << "Muzzley plugs started!" << endl << flush;

        //Starts the UPNP advertisement
        std::thread upnp_thread(upnp_advertise);
        upnp_thread.detach();

        //Lists all alljoyn devices periodically
        std::thread alljoyn_status_thread(alljoyn_status_info);
        alljoyn_status_thread.detach();

        //Cleans all timmed out muzzley read requstes
        std::thread muzzley_read_request_cleaner_thread(muzzley_read_request_cleaner);
        muzzley_read_request_cleaner_thread.detach();

        //Get all Available LampsIDs for upnp server
        status = lampManager.GetAllLampIDs();
        muzzley_query_unknown_lampnames(&lampManager);
    
        while(true){}
    
        cleanup();

    }catch(exception& e){
        cout << "Error: " << e.what() << endl << flush;
        return true;
    }

    return false;
}
