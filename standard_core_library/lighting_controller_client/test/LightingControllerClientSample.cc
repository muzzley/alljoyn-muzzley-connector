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

//14.12
//#include <alljoyn/ProxyBusObject.h>
//#include <ProxyBusObject.h>

//#define MUZZLEY_STAGING

#if defined(MUZZLEY_STAGING)
	#define MUZZLEY_PLUGS_APP_TOKEN "cd2b5e70beca9e9f"
	#define MUZZLEY_PLUGS_PROFILEID "54a3e58e93ffd78eeea86fcb"
	#define MUZZLEY_LIGHTING_APP_TOKEN "bc3f07fae0ff182c"
	#define MUZZLEY_LIGHTING_PROFILEID "546f5f2d006233c726739de3"
	#define MUZZLEY_ENDPOINTHOST "platform.office.muzzley.com"
	#define MUZZLEY_API_ENDPOINTHOST "channel-api.office.muzzley.com"
	#define MUZZLEY_MANAGER_ENDPOINTHOST "global-manager.office.muzzley.com"
#else
	#define MUZZLEY_PLUGS_APP_TOKEN "b640b74e19f831a3"
	#define MUZZLEY_PLUGS_PROFILEID "54aaee6cec3302c4272076a2"
	#define MUZZLEY_LIGHTING_APP_TOKEN "5f09fd4b3b6f8821"
	#define MUZZLEY_LIGHTING_PROFILEID "5486e2e3ec3302c42720738b"
	#define MUZZLEY_ENDPOINTHOST "geoplatform.muzzley.com"
	#define MUZZLEY_API_ENDPOINTHOST "channels.muzzley.com"
	#define MUZZLEY_MANAGER_ENDPOINTHOST "global-manager.muzzley.com"
#endif

#define MUZZLEY_MANAGER_REGISTER_URL "/deviceapp/register"
#define MUZZLEY_MANAGER_COMPONENTS_URL "/deviceapp/components"
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

#define COLOR_MIN 0
#define COLOR_MAX_UINT32 4294967296
#define COLOR_MAX_PERCENT 100
#define COLOR_MAX_360DEG 360
#define COLOR_TEMPERATURE_MIN_UINT32 384286547
#define COLOR_TEMPERATURE_MAX_UINT32 904203641
#define COLOR_TEMPERATURE_MIN_DEC 2700
#define COLOR_TEMPERATURE_MAX_DEC 5000
#define COLOR_TEMPERATURE_DEFAULT_DEC 3600
#define COLOR_RGB_MAX 255
#define COLOR_DOUBLE_MAX 1

#define GUPNP_MAX_AGE 1800
#define GUPNP_MESSAGE_DELAY 120

#define DEVICE_PLUG "plug"
#define DEVICE_BULB "bulb"
#define DEVICE_BRIDGE "bridge"
#define PROPERTY_STATUS "status"
#define PROPERTY_COLOR "color"
#define PROPERTY_BRIGHTNESS "brightness"
#define PROPERTY_VOLTAGE "voltage"
#define PROPERTY_CURRENT "current"
#define PROPERTY_FREQUENCY "freq"
#define PROPERTY_POWER "power"
#define PROPERTY_ENERGY "energy"

#define MUZZLEY_LIGHTING_FRIENDLYNAME "Alljoyn Lighting"
#define MUZZLEY_PLUGS_FRIENDLYNAME "Alljoyn Plugs"
#define MUZZLEY_MANUFACTURER "Muzzley"
#define MUZZLEY_MANUFACTURER_URL "www.muzzley.com"
#define MUZZLEY_MODELDESCRIPTION "Muzzley Alljoyn Connector with UPnP Support"
#define MUZZLEY_MODELNAME "Muzzley Connector 0.1"
#define MUZZLEY_MODELNUMBER "muzzley-modelnumber"
#define MUZZLEY_LIGHTING_UDN "muzzley-lighting-udn"
#define MUZZLEY_PLUGS_UDN "muzzley-plugs-udn"
#define MUZZLEY_SERIALNUMBER_LIGHTING "muzzley-lighting-serialnumber-lisbon-office-1"
#define MUZZLEY_SERIALNUMBER_PLUGS "muzzley-plugs-serialnumber-lisbon-office-1"


using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

using namespace qcc;
using namespace lsf;
using namespace ajn;
using namespace services;

bool connectedToControllerService = false;
LSFStringList lampList;

AnnounceHandlerImpl* announceHandler = 0;
ControlPanelService* controlPanelService = 0;
ControlPanelController* controlPanelController = 0;
ControlPanelListenerImpl* controlPanelListener = 0;
NotificationService* conService = 0;
NotificationService* notificationService = 0;
NotificationSender* notificationSender = 0;
ControllerNotificationReceiver* controller_receiver = 0;

BusAttachment* bus;

string muzzley_macAddress = "AABBCCDDEEFF";
string muzzley_lighting_deviceKey;
string muzzley_plugs_deviceKey;
bool muzzley_lighting_registered=false;
bool muzzley_plugs_registered=false;
LSFString muzzley_controllerServiceID;
LSFString muzzley_controllerServiceName;

//Component/property/CID/t/time/type
typedef tuple <string, string, string, int, time_t, string> muzzley_req;
vector <muzzley_req> req_vec;

//component/label/status/voltage/current/freq/watt/accu/GetProperties/On/Off/time
typedef tuple <string, string, Property*, Property*, Property*, Property*, Property*, Property*, Action*, Action*, Action*, time_t> alljoyn_plug;
vector <alljoyn_plug> plug_vec;

//lampID/lampName
unordered_map <string, string> muzzley_lamplist;

muzzley::Client _muzzley_plugs_client;


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

    *v = max;                    // v
    delta = max - min;
    if( max != 0 )
        *s = delta / max;        // s
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


void print_plug_vector(){
	try{
		for (unsigned int i = 0; i < plug_vec.size(); i++){
			cout << endl << "Plug Vector Pos#: " << i << " of# : " << req_vec.size() << endl << flush;
			cout << "Component: " << get<0>(plug_vec[i]) << endl << flush;
			cout << "Label: " << get<1>(plug_vec[i]) << endl << endl << flush;
		}
	}catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
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

void print_request_vector(){
	for (unsigned int i = 0; i < req_vec.size(); i++){
		cout << endl << "Vector Pos#: " << i << " of# : " << req_vec.size() << endl << flush;
		cout << "Component: " << get<0>(req_vec[i]) << endl << flush;
		cout << "Property: " << get<1>(req_vec[i]) << endl << flush;
		cout << "CID: " << get<2>(req_vec[i]) << endl << flush;
		cout << "T: " << get<3>(req_vec[i]) << endl << flush;
		cout << "Time: " << get<4>(req_vec[i]) << endl << flush;
		cout << "Type: " << get<5>(req_vec[i]) << endl << endl << flush;
	}
}

int get_request_vector_pos(string component, string property){
	try{
		for (unsigned int i = 0; i < req_vec.size(); i++){
			if(get<0>(req_vec[i])==component){
				if(get<1>(req_vec[i])==property){
					cout << "Muzzley request found on pos: " << i << endl << flush; 
					return i;
				}
			}
		}
		cout << "Muzzley request not found" << endl << flush; 
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
		cout << "Muzzley read request#: " << i << " duration: " << req_duration << endl << endl << flush;
		if(req_duration>MUZZLEY_READ_REQUEST_TIMEOUT){
	        req_vec.erase(req_vec.begin()+i);
	        cout << "Erased muzzley read request#: " << i << " (timeout!)" << endl << flush;
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
            if(local_it->first ==  lampID){
				cout << endl << "Updating lamp name for id: " << local_it->first << endl << flush;
				local_it->second=lampName;
				return true;
            }
        }
    }
    return false;
}

string muzzley_lamplist_get_lampname(string lampID){
    for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
        for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
            if(local_it->first == lampID){
				cout << endl << "Found lamp id: " << local_it->first << endl << flush;
				if(local_it->second != MUZZLEY_UNKNOWN_NAME && local_it->second != ""){
					cout << endl << "lampname: " << local_it->second << endl << flush;
					return local_it->second;
				}
				else
					return MUZZLEY_UNKNOWN_NAME;
            }
        }
    }
    return MUZZLEY_UNKNOWN_NAME;
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
        responseStream << "<deviceType>urn:Muzzley:device:" << MUZZLEY_LIGHTING_PROFILEID << ":1</deviceType>\n";
        responseStream << "<friendlyName>" << MUZZLEY_LIGHTING_FRIENDLYNAME << "</friendlyName>\n";
        responseStream << "<manufacturer>" << MUZZLEY_MANUFACTURER << "</manufacturer>\n";
        responseStream << "<manufacturerURL>" << MUZZLEY_MANUFACTURER_URL << "</manufacturerURL>\n";
        responseStream << "<modelDescription>" << MUZZLEY_MODELDESCRIPTION << "</modelDescription>\n";
        responseStream << "<modelName>" << MUZZLEY_MODELNAME << "</modelName>\n";
        responseStream << "<modelNumber>" << MUZZLEY_MODELNUMBER << "</modelNumber>\n";
        responseStream << "<UDN>uuid:" << MUZZLEY_LIGHTING_UDN << "</UDN>\n";
        responseStream << "<serialNumber>" << MUZZLEY_SERIALNUMBER_LIGHTING << "</serialNumber>\n";
        responseStream << "<macAddress>" << muzzley_macAddress << "</macAddress>\n";
        responseStream << "<deviceKey>" << muzzley_lighting_deviceKey << "</deviceKey>\n";
        responseStream << "<components>\n";
            if(MUZZLEY_BRIDGE_INFO){
                if(muzzley_controllerServiceID!=""){
                responseStream << "<component>\n";
                    responseStream << "<id>";
                    responseStream << muzzley_controllerServiceID;
                    responseStream << "</id>\n";

                    responseStream << "<label>";
                    responseStream << muzzley_controllerServiceName;
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
    _socket.open(MUZZLEY_API_ENDPOINTHOST, 80); //HTTPS:443

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPGet);

    // set HTTP request server path
    std::stringstream url;
    url << "/profiles/" << MUZZLEY_LIGHTING_PROFILEID;
    _req->url(url.str());
    _req->header("Host", MUZZLEY_API_ENDPOINTHOST);
    _req->header("Accept", "*/*");

    _socket << _req << flush;
    cout << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;

    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _url = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << endl << "Parsed Muzzley API Reply:" << endl << flush;
        cout << "id: " << (string)_url["id"] << endl << flush;
        cout << "uuid: " << (string)_url["uuid"] << endl << flush;
        cout << "name: " << (string)_url["name"] << endl << flush;
        cout << "provider: " << (string)_url["provider"] << endl << flush;
        cout << "deviceHandlerUrl: " << (string)_url["deviceHandlerUrl"] << endl << endl << flush;
        string str_url=(string)_url["deviceHandlerUrl"];

    }else{
        cout << "Error: " << _rep->status() << endl << flush;
    }

    _socket.close();
    return true;
}

bool muzzley_lighting_connect_manager(){

	// Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);

    muzzley_lighting_deviceKey=muzzley_read_lighting_deviceKey_file();
    cout << "File lighting device key: " << muzzley_lighting_deviceKey << endl << flush;

    // Instantiate a string with some body part
    muzzley::JSONObj _json_body_part;
        _json_body_part <<
            "profileId" << MUZZLEY_LIGHTING_PROFILEID <<
            "macAddress" << muzzley_macAddress <<
            "serialNumber" << MUZZLEY_SERIALNUMBER_LIGHTING <<
            "friendlyName" << MUZZLEY_LIGHTING_FRIENDLYNAME;

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
    _req->url(MUZZLEY_MANAGER_REGISTER_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());

    _req->body(_str_body_part);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);


    muzzley::JSONArr _components;
    if(MUZZLEY_BRIDGE_INFO){
        muzzley::JSONObj _bridge = JSON(
            "id" << muzzley_controllerServiceID <<
            "label" << muzzley_controllerServiceName <<
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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_replace_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);

    _req->body(_str_add_component);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_add_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_del_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    responseStream << "<deviceType>urn:Muzzley:device:" << MUZZLEY_PLUGS_PROFILEID << ":1</deviceType>\n";
    responseStream << "<friendlyName>" << MUZZLEY_PLUGS_FRIENDLYNAME << "</friendlyName>\n";
    responseStream << "<manufacturer>" << MUZZLEY_MANUFACTURER << "</manufacturer>\n";
    responseStream << "<manufacturerURL>" << MUZZLEY_MANUFACTURER_URL << "</manufacturerURL>\n";
    responseStream << "<modelDescription>" << MUZZLEY_MODELDESCRIPTION << "</modelDescription>\n";
    responseStream << "<modelName>" << MUZZLEY_MODELNAME << "</modelName>\n";
    responseStream << "<modelNumber>" << MUZZLEY_MODELNUMBER << "</modelNumber>\n";
    responseStream << "<UDN>uuid:" << MUZZLEY_PLUGS_UDN << "</UDN>\n";
    responseStream << "<serialNumber>" << MUZZLEY_SERIALNUMBER_PLUGS << "</serialNumber>\n";
    responseStream << "<macAddress>" << muzzley_macAddress << "</macAddress>\n";
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
    _socket.open(MUZZLEY_API_ENDPOINTHOST, 80); //HTTPS:443

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPGet);

    // set HTTP request server path
    std::stringstream url;
    url << "/profiles/" << MUZZLEY_PLUGS_PROFILEID;
    _req->url(url.str());
    _req->header("Host", MUZZLEY_API_ENDPOINTHOST);
    _req->header("Accept", "*/*");

    _socket << _req << flush;
    cout << _req << endl << flush;

    // Instantiate an HTTP response object
    muzzley::HTTPRep _rep;
    _socket >> _rep;

    if (_rep->status() == muzzley::HTTP200 || _rep->status() == muzzley::HTTP201) {
        // Print the value of a message header
        muzzley::JSONObj _url = (muzzley::JSONObj&) muzzley::fromstr(_rep->body());
        cout << endl << "Parsed Muzzley API Reply:" << endl << flush;
        cout << "id: " << (string)_url["id"] << endl << flush;
        cout << "uuid: " << (string)_url["uuid"] << endl << flush;
        cout << "name: " << (string)_url["name"] << endl << flush;
        cout << "provider: " << (string)_url["provider"] << endl << flush;
        cout << "deviceHandlerUrl: " << (string)_url["deviceHandlerUrl"] << endl << endl << flush;
        string str_url=(string)_url["deviceHandlerUrl"];

    }else{
        cout << "Error: " << _rep->status() << endl << flush;
    }

    _socket.close();
    return true;
}

bool muzzley_plugs_connect_manager(){

	// Instantiate an HTTP(s) socket stream
    muzzley::socketstream _socket;
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

    // Instantiate an HTTP request object
    muzzley::HTTPReq _req;
    _req->method(muzzley::HTTPPost);

    muzzley_plugs_deviceKey=muzzley_read_plugs_deviceKey_file();
    cout << "File plugs device key: " << muzzley_plugs_deviceKey << endl << flush;

    // Instantiate a string with some body part
    muzzley::JSONObj _json_body_part;
        _json_body_part <<
            "profileId" << MUZZLEY_PLUGS_PROFILEID <<
            "macAddress" << muzzley_macAddress <<
            "serialNumber" << MUZZLEY_SERIALNUMBER_PLUGS <<
            "friendlyName" << MUZZLEY_PLUGS_FRIENDLYNAME;

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
    _req->url(MUZZLEY_MANAGER_REGISTER_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());

    _req->body(_str_body_part);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_PLUGS);
    _req->header("DEVICEKEY", muzzley_plugs_deviceKey);

    _req->body(_str_replace_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);

    _req->body(_str_add_component);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_add_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
    _socket.open(MUZZLEY_MANAGER_ENDPOINTHOST, 80);

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
    _req->url(MUZZLEY_MANAGER_COMPONENTS_URL);
    _req->header("Host", MUZZLEY_MANAGER_ENDPOINTHOST);
    _req->header("Accept", "*/*");
    _req->header("Content-Type", "application/json");
    _req->header("Content-Length", ss.str());
    _req->header("SERIALNUMBER", MUZZLEY_SERIALNUMBER_LIGHTING);
    _req->header("DEVICEKEY", muzzley_lighting_deviceKey);


    _req->body(_str_del_components);
    _socket << _req << flush;
    cout << _req << endl << flush;

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
        _s1.setProfile(MUZZLEY_LIGHTING_PROFILEID);
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
	           "profile" << MUZZLEY_LIGHTING_PROFILEID <<
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
        _s1.setProfile(MUZZLEY_LIGHTING_PROFILEID);
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
	            "profile" << MUZZLEY_LIGHTING_PROFILEID <<
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
        _s1.setProfile(MUZZLEY_LIGHTING_PROFILEID);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent(lampID);
        _s1.setProperty(PROPERTY_COLOR);


        muzzley::Message _m1;

        int pos = get_request_vector_pos(lampID, PROPERTY_COLOR);
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
	          	"profile" << MUZZLEY_LIGHTING_PROFILEID <<
	            "channel" << muzzley_lighting_deviceKey <<
	            "component" << lampID <<
	            "property" << PROPERTY_COLOR <<
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

	    long long brightness_int = color_remap_long_long(long_brightness, COLOR_MIN,    (COLOR_MAX_UINT32-1),    COLOR_MIN,    COLOR_MAX_PERCENT);
	    long long hue_int        = color_remap_long_long(long_hue,        COLOR_MIN,    (COLOR_MAX_UINT32-1),    COLOR_MIN,    COLOR_MAX_360DEG);
	    long long saturation_int = color_remap_long_long(long_saturation, COLOR_MIN,    (COLOR_MAX_UINT32-1),    COLOR_MIN,    COLOR_MAX_PERCENT);
	    long long colortemp_int  = color_remap_long_long(long_colortemp,  COLOR_TEMPERATURE_MIN_UINT32, COLOR_TEMPERATURE_MAX_UINT32, COLOR_TEMPERATURE_MIN_DEC, COLOR_TEMPERATURE_MAX_DEC);

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
        
        if(muzzley_publish_lampColor_rgb(lampID, red, green, blue, _muzzley_lighting_client)==false){
            cout  << "Error publishing lampcolorRGB to muzzley" << endl << flush;
        }else{
            cout  << "Published lampcolorRGB to muzzley sucessfully" << endl << flush;
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
            cout << "LampManager Error: " << status << endl << flush;
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
		    cout << "LampManager Error: " << status << endl << flush;
		return true;
	}catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}


bool muzzley_handle_lighting_write_RGB_request(LampManager& lampManager, string component, int red, int green, int blue){
	int status;
	try{
	    double red_double   = color_remap_double(red,   COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);
	    double green_double = color_remap_double(green, COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);
	    double blue_double  = color_remap_double(blue,  COLOR_MIN, COLOR_RGB_MAX, COLOR_MIN, COLOR_DOUBLE_MAX);

	    double hue_double, saturation_double, value_double;
	    RGBtoHSV(red_double, green_double, blue_double, &hue_double, &saturation_double, &value_double);

	    long long long_hue = color_remap_long_long(round(hue_double), COLOR_MIN, COLOR_MAX_360DEG, COLOR_MIN, (COLOR_MAX_UINT32-1));
	    long long long_saturation = color_remap_long_long(round(saturation_double), COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, (COLOR_MAX_UINT32-1));
	    long long long_brightness = color_remap_long_long(round(value_double), COLOR_MIN, COLOR_DOUBLE_MAX, COLOR_MIN, (COLOR_MAX_UINT32-1));

        printf("Received Red int: %d\n", red);
	    printf("Received Green int: %d\n", green);
	    printf("Received Blue int: %d\n", blue);
	    printf("Calculated hue double: %f\n", hue_double);
	    printf("Calculated saturation double: %f\n", saturation_double);
	    printf("Calculated value double: %f\n", value_double);
	    printf("Brightness: %lld\n", long_brightness);
	    printf("Hue: %lld\n", long_hue);
	    printf("Saturation: %lld\n", long_saturation);
	    
		//onoff/Hue/Saturation/Colortemp/Brightness
	    LampState state(true, long_hue, long_saturation, COLOR_TEMPERATURE_DEFAULT_DEC, long_brightness);
	    status = lampManager.TransitionLampState(component, state);
	    if(status != LSF_OK)
	        cout << "LampManager Error: " << status << endl << flush;
	    return true; 
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
            cout << "LampManager Error: " << status << endl << flush;
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
    string cid = (string)_data["h"]["cid"];
    int t = (int)_data["h"]["t"];

    muzzley_query_unknown_lampnames(&lampManager);
    print_request_vector();

    if (io=="r"){
    	muzzley_handle_lighting_read_request(lampManager, component, property, cid, t);
    }
    if (io=="w"){
        if(property=="status"){
            bool bool_status = (bool)_data["d"]["p"]["data"]["value"];
            muzzley_handle_lighting_write_status_request(lampManager, component, bool_status);
        }
        if(property==PROPERTY_BRIGHTNESS){
            double brightness = (double)_data["d"]["p"]["data"]["value"];
            muzzley_handle_lighting_write_brightness_request(lampManager, component, brightness);   
        }
        if(property==PROPERTY_COLOR){
            int red   = (int)_data["d"]["p"]["data"]["value"]["r"];
            int green = (int)_data["d"]["p"]["data"]["value"]["g"];
            int blue  = (int)_data["d"]["p"]["data"]["value"]["b"];
            muzzley_handle_lighting_write_RGB_request(lampManager, component, red,  green,  blue);                                
        }
	}
	return true;
}



bool muzzley_publish_plug_state(string plugID, bool onoff){
    try{

    	int semaphore = semaphore_start();

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(MUZZLEY_PLUGS_PROFILEID);
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
	           "profile" << MUZZLEY_PLUGS_PROFILEID <<
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
        _s1.setProfile(MUZZLEY_PLUGS_PROFILEID);
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
	           "profile" << MUZZLEY_PLUGS_PROFILEID <<
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
    string cid = (string)_data["h"]["cid"];
    int t = (int)_data["h"]["t"];
    
    cout << "Handling plug request for ID: " << component << endl << flush;
    print_request_vector();
    print_plug_vector();
    
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
        printf("\n%s:\ncontrollerServiceDeviceID = %s\ncontrollerServiceName = %s\n\n", __func__, uniqueId.data(), name.data());
        muzzley_controllerServiceID=uniqueId;
        muzzley_controllerServiceName=name;
        connectedToControllerService = true;
    }

    void ConnectToControllerServiceFailedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        printf("\n%s:\ncontrollerServiceDeviceID = %s\ncontrollerServiceName = %s\n\n", __func__, uniqueId.data(), name.data());
    }

    void DisconnectedFromControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        printf("\n%s:\ncontrollerServiceDeviceID = %s\ncontrollerServiceName = %s\n\n", __func__, uniqueId.data(), name.data());
        muzzley_controllerServiceID="";
        muzzley_controllerServiceName="";
        connectedToControllerService = false;
    }

    void ControllerClientErrorCB(const ErrorCodeList& errorCodeList) {
        printf("\n%s:", __func__);
        ErrorCodeList::const_iterator it = errorCodeList.begin();
        for (; it != errorCodeList.end(); ++it) {
            printf("\n%s", ControllerClientErrorText(*it));
        }
        printf("\n");
    }
    

};

class ControllerServiceManagerCallbackHandler : public ControllerServiceManagerCallback {


    void ControllerServiceLightingResetCB(void) {
        printf("\n%s\n", __func__);
    }
    
    void ControllerServiceNameChangedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString uniqueId = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        printf("\n%s:\ncontrollerServiceDeviceID = %s\ncontrollerServiceName = %s\n\n", __func__, uniqueId.data(), name.data());
        muzzley_controllerServiceID=controllerServiceDeviceID;
        muzzley_controllerServiceName=controllerServiceName;
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
        printf("\n%s():\nresponseCode = %s\nlistsize=%lu", __func__, LSFResponseCodeText(responseCode), lampIDs.size());
        if (responseCode == LSF_OK) {
            muzzley_lamplist.clear();
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
        printf("\n%s:\nresponseCode = %s\nlampID = %s\nlanguage = %s\n", __func__, LSFResponseCodeText(responseCode), uniqueId.data(), language.data());
        if (responseCode == LSF_OK) {
            printf("lampName = %s\n\n", lampName.data());
            for ( unsigned i = 0; i < muzzley_lamplist.bucket_count(); ++i) {
                for ( auto local_it = muzzley_lamplist.begin(i); local_it!= muzzley_lamplist.end(i); ++local_it ){
                    if(local_it->first == lampID.data() ){
                        local_it->second = lampName.data();
                        cout << endl << "Lamp ID: " << lampID << endl << "Name: " << lampName << endl << "was updated in lamplist" << endl << endl << flush; 
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
        printf("\n%s:\nresponseCode = %s\nlampID = %s", __func__, LSFResponseCodeText(responseCode), uniqueId.data());
        if (responseCode == LSF_OK) {
            printf("\nstate=%s\n", lampState.c_str());
            muzzley_parseLampState(lampID, lampState, this->_client);
        }
    }

    void LampStateChangedCB(const LSFString& lampID, const LampState& lampState) {
        printf("\n%s:\nlampID = %s\nlampState = \n%s", __func__, lampID.data(), lampState.c_str());
        muzzley_parseLampState(lampID, lampState, this->_client);
    }

    void LampsFoundCB(const LSFStringList& lampIDs) {
        printf("\n%s():\nlistsize=%lu", __func__, lampIDs.size());
        LSFStringList::const_iterator it = lampIDs.begin();
        uint8_t count = 1;
        for (; it != lampIDs.end(); ++it) {
            printf("\n(%d)%s", count, (*it).data());
            count++;
            muzzley_lamplist[(*it).data()]=muzzley_lamplist_get_lampname((*it).data());
            muzzley_add_lighting_components(lampIDs);
        }
        printf("\n");
        lampList.clear();
        lampList = lampIDs;
    }

    void LampsLostCB(const LSFStringList& lampIDs) {
        printf("\n%s():\nlistsize=%lu", __func__, lampIDs.size());
        LSFStringList::const_iterator it = lampIDs.begin();
        uint8_t count = 1;
        for (; it != lampIDs.end(); ++it) {
            printf("\n(%d)%s", count, (*it).data());
            count++;
            muzzley_remove_lighting_components(lampIDs);
        }
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
		    	
		    	//CONTAINER==0
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
										cout << "	Widget Type: " << widgetType << endl << flush;
											switch(widgetType){
												case WIDGET_TYPE_ACTION:
													cout << "	Widget Type: ACTION" << endl << flush;
													if(label=="On")
														plug_action_on=((Action*)childchildWidgets[j]);
													else if (label=="Off")
														plug_action_off=((Action*)childchildWidgets[j]);
													else if (label=="Get Properties")
														plug_action_get_properties=((Action*)childchildWidgets[j]);
										            break;
												case WIDGET_TYPE_ACTION_WITH_DIALOG:
													cout << "	Widget Type: ACTION_WITH_DIALOG" << endl << flush;
										            break;
										        case WIDGET_TYPE_LABEL:
										        	cout << "	Widget Type: LABEL" << endl << flush;
										            break;
										        case WIDGET_TYPE_PROPERTY:
										        	cout << "	Widget Type: PROPERTY" << endl << flush;
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
										        	cout << "	Widget Type: CONTAINER" << endl << flush;
										            break;
										        case WIDGET_TYPE_DIALOG:
										        	cout << "	Widget Type: DIALOG" << endl << flush;
										            break;
										        case WIDGET_TYPE_ERROR:
										        	cout << "	Widget Type: ERROR" << endl << flush;
										            break;
										        
											}
											cout << "	Label: " << label << endl << flush;
											cout << "	Secured: " << secured << endl << flush;
											cout << "	Enabled: " << enabled << endl << flush;
											cout << "	Writable: " << writable << endl << flush;
											cout << "	States: " << states << endl << flush;
											cout << "	BGcolor: " << bgcolor << endl << endl << flush;
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
				
			    //DIALOG==1?
			    else if (rootContainer->getWidgetType() == 1) {
			    	cout << "AnnounceHandler Widget type->DIALOG" << endl;
			    } else {
			        std::cout << "AnnounceHandler RootWidget is of unknown type" << std::endl;
			    }
			    

			}
	    }

	//Working...
	//plug_vec.push_back(make_tuple(device_id_str, device_name_str, plug_property_status, plug_property_volt, plug_property_curr, plug_property_freq, plug_property_watt , plug_property_accu, plug_action_get_properties, plug_action_on, plug_action_off));
	del_plug_vector_pos(device_id_str);
	add_plug_vector_pos(device_id_str, device_name_str, plug_property_status, plug_property_volt, plug_property_curr, plug_property_freq, plug_property_watt , plug_property_accu, plug_action_get_properties, plug_action_on, plug_action_off);
	muzzley_add_plugs_component(device_id_str, device_name_str);
	gupnp_generate_plugs_XML();
	print_plug_vector();
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


int main(){
	
	if(muzzley_check_semaphore_file_exists(MUZZLEY_SEMAPHORE_FILENAME))
		return true;

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
    
   

    //Muzzley Protocol
    _muzzley_lighting_client.setEndpointHost(MUZZLEY_ENDPOINTHOST);
    _muzzley_lighting_client.setLoopAssynchronous(MUZZLEY_LOOPASSYNCHRONOUS);
    _muzzley_plugs_client.setEndpointHost(MUZZLEY_ENDPOINTHOST);
    _muzzley_plugs_client.setLoopAssynchronous(MUZZLEY_LOOPASSYNCHRONOUS);

    _muzzley_lighting_client.on(muzzley::AppLoggedIn,[&lampManager] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool{
        cout << "Lighting logged in with id " << _data["d"]["activityId"] << endl << flush;

        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(MUZZLEY_LIGHTING_PROFILEID);
        _s1.setChannel(muzzley_lighting_deviceKey);
        _s1.setComponent("*");
        _s1.setProperty("*");

        _muzzley_lighting_client.on(muzzley::Published, _s1, [&lampManager] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool {
            //_data->prettify(cout);
            //cout << endl << flush;
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

	_muzzley_plugs_client.on(muzzley::AppLoggedIn,[] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool{
        cout << "Plugs logged in with id " << _data["d"]["activityId"] << endl << flush;
       
        muzzley::Subscription _s1;
        _s1.setNamespace(MUZZLEY_WORKSPACE);
        _s1.setProfile(MUZZLEY_PLUGS_PROFILEID);
        _s1.setChannel(muzzley_plugs_deviceKey);
        _s1.setComponent("*");
        _s1.setProperty("*");

        _muzzley_plugs_client.on(muzzley::Published, _s1, [] (muzzley::Message& _data, muzzley::Client& _muzzley_lighting_client) -> bool {
            //_data->prettify(cout);
            //cout << endl << flush;
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
    
    try{
        // Waits for global manager lighting devicekey
        while(!muzzley_lighting_registered){
			muzzley_lighting_connect_API();
	        muzzley_lighting_registered=muzzley_lighting_connect_manager();
        	printf("Waiting for Muzzley lighting registration...\n");
        	sleep(1);
        }

        // Waits for global manager plugs devicekey
        while(!muzzley_lighting_registered || !muzzley_plugs_registered){
        	muzzley_plugs_connect_API();
        	muzzley_plugs_registered=muzzley_plugs_connect_manager();     
        	printf("Waiting for Muzzley plugs registration...\n");
        	sleep(1);
        }

    	//Connects the application to the Muzzley server.
        _muzzley_lighting_client.initApp(MUZZLEY_LIGHTING_APP_TOKEN);
        cout << "Muzzley lighting started!" << endl << flush;

        //Connects the application to the Muzzley server.
        _muzzley_plugs_client.initApp(MUZZLEY_PLUGS_APP_TOKEN);
        cout << "Muzzley plugs started!" << endl << flush;

        //Get All Available LampsIDs for upnp server
        status = lampManager.GetAllLampIDs();
        sleep(1);
		
		muzzley_query_unknown_lampnames(&lampManager);
	
		//UPnP
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
		gupnp_lighting_context = gupnp_context_new (NULL, NULL, 0, &error);
		if (error) {
			g_printerr ("Error creating the GUPnP lighting context: %s\n", error->message);
			g_error_free (error);
			return EXIT_FAILURE;
		}
		
		muzzley_query_unknown_lampnames(&lampManager);
		gupnp_generate_lighting_XML();

		gupnp_lighting_dev = gupnp_root_device_new (gupnp_lighting_context, MUZZLEY_LIGHTING_XML_FILENAME, ".");
		gupnp_root_device_set_available (gupnp_lighting_dev, TRUE);
		gupnp_lighting_resource_group = gupnp_root_device_get_ssdp_resource_group(gupnp_lighting_dev);
		gssdp_resource_group_set_max_age (gupnp_lighting_resource_group, GUPNP_MAX_AGE);
		gssdp_resource_group_set_message_delay(gupnp_lighting_resource_group, GUPNP_MESSAGE_DELAY);

	    std::stringstream lighting_device_urn;
	    lighting_device_urn << "urn:Muzzley:device:" << MUZZLEY_LIGHTING_PROFILEID << ":1";
	    const std::string tmp_lighting_urn = lighting_device_urn.str();
		const char* lighting_device_urn_char = tmp_lighting_urn.c_str();    
	   
	    std::stringstream lighting_host;
		lighting_host << "http://" << gupnp_context_get_host_ip (gupnp_lighting_context) << ":" << gupnp_context_get_port (gupnp_lighting_context) << "/" << gupnp_root_device_get_relative_location(gupnp_lighting_dev);
		const char* lighting_host_char = lighting_host.str().c_str(); 

		cout << endl << "GUPNP LIGHTING INFO:" << endl << flush;
		cout << "URN: " << lighting_device_urn_char << endl << flush;
		cout << "HOST: " << lighting_host_char << endl << flush;
		cout << "XML FILE PATH: " << gupnp_root_device_get_description_path(gupnp_lighting_dev) << endl << flush;
		cout << "RELATIVE LOCATION: " << gupnp_root_device_get_relative_location(gupnp_lighting_dev) << endl << flush;
		cout << "PORT: " << gupnp_context_get_port (gupnp_lighting_context) << endl << flush;

		gssdp_resource_group_add_resource_simple(gupnp_lighting_resource_group, lighting_device_urn_char, lighting_device_urn_char , lighting_host_char ); 	  	  //urn:urn
		gssdp_resource_group_set_available (gupnp_lighting_resource_group, TRUE);

		//Plugs UPnP
		gupnp_plugs_context = gupnp_context_new (NULL, NULL, 0, &error);
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
	    plugs_device_urn << "urn:Muzzley:device:" << MUZZLEY_PLUGS_PROFILEID << ":1";
	    const std::string tmp_plugs_urn = plugs_device_urn.str();
		const char* plugs_device_urn_char = tmp_plugs_urn.c_str();    
	   
	    std::stringstream plugs_host;
		plugs_host << "http://" << gupnp_context_get_host_ip (gupnp_plugs_context) << ":" << gupnp_context_get_port (gupnp_plugs_context) << "/" << gupnp_root_device_get_relative_location(gupnp_plugs_dev);
		const char* plugs_host_char = plugs_host.str().c_str(); 

		cout << endl << "GUPNP PLUGS INFO:" << endl << flush;
		cout << "URN: " << plugs_device_urn_char << endl << flush;
		cout << "HOST: " << plugs_host_char << endl << flush;
		cout << "XML FILE PATH: " << gupnp_root_device_get_description_path(gupnp_plugs_dev) << endl << flush;
		cout << "RELATIVE LOCATION: " << gupnp_root_device_get_relative_location(gupnp_plugs_dev) << endl << flush;
		cout << "PORT: " << gupnp_context_get_port (gupnp_plugs_context) << endl << endl << flush;

		gssdp_resource_group_add_resource_simple(gupnp_plugs_resource_group, plugs_device_urn_char, plugs_device_urn_char , plugs_host_char ); 	  	  //urn:urn
		gssdp_resource_group_set_available (gupnp_plugs_resource_group, TRUE);

		// Run the main loop
	    main_loop = g_main_loop_new (NULL, FALSE);
	    g_main_loop_run (main_loop);
	  	
	    
	    cleanup();

    }catch(exception& e){
    	cout << "Error: " << e.what() << endl << flush;
    	return true;
    }

    return false;
}
