//Vehicle Management Source
//	Created By:		Mike Moss
//	Modified On:	08/26/2013

//IO Stream Header
#include <iostream>

//JSON Header
#include "msl/json.hpp"

//Serial Sync Header
#include "SerialSync.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//String Stream Header
#include <sstream>

//String Utility Header
#include "msl/string_util.hpp"

//Vector Header
#include <vector>

//Web Server Header
#include "msl/webserver.hpp"

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

//Vehicle Class
class vehicle
{
	public:
		//Constructor (Default)
		vehicle(const int ID=-1,const std::string& RADIO="",const int BAUD=57600):
			id(ID),radio(RADIO),baud(BAUD),x(0),y(0),direction(0),dx(-1),dy(-1),
			enc0a(0),enc0b(0),enc1a(0),enc1b(0),ir0(0),ir1(0),us0(0),us1(0),
			accel_x(0),accel_y(0),accel_z(0),_sync(radio,baud)
		{}

		//Update Function
		void update()
		{
			//Get Updated Sensor Values
			if(_sync.good())
			{
				enc0a=_sync.get(2);
				enc0b=_sync.get(3);
				enc1a=_sync.get(4);
				enc1b=_sync.get(5);
				ir0=_sync.get(6);
				ir1=_sync.get(7);
				us0=_sync.get(8);
				us1=_sync.get(9);
				accel_x=_sync.get(9);
				accel_y=_sync.get(10);
				accel_z=_sync.get(11);
			}

			//MOVE HERE, RIGHT NOW I'M JUST KILLING THE MOTORS
			if(_sync.good())
			{
				_sync.set(0,0);
				_sync.set(1,0);
			}
		}

		//JSON Function (Returns Object as a JSON Object)
		msl::json json() const
		{
			//Create Temporary JSON Object
			msl::json temp;

			//Set Temporary JSON Object's Variables
			temp.set("id",id);
			temp.set("radio",radio);
			temp.set("baud",baud);
			temp.set("connected",_sync.good());
			temp.set("x",x);
			temp.set("y",y);
			temp.set("direction",direction);
			temp.set("dx",dx);
			temp.set("dy",dy);
			temp.set("enc0a",enc0a);
			temp.set("enc0b",enc0b);
			temp.set("enc1a",enc1a);
			temp.set("enc1b",enc1b);
			temp.set("ir0",ir0);
			temp.set("ir1",ir1);
			temp.set("us0",us0);
			temp.set("us1",us1);

			//Return Temporary JSON Object
			return temp;
		}

		//Connect Function (Starts Serial Port Communication)
		void connect()
		{
			try
			{
				_sync.stop();
				_sync.setup();
			}
			catch(...)
			{}
		}

		//Disconnect Function (Stops Serial Port Communication)
		void disconnect()
		{
			_sync.stop();
		}

		//Public Member Variables
		int id;
		std::string radio;
		int baud;
		int x;
		int y;
		int direction;
		int dx;
		int dy;
		int enc0a;
		int enc0b;
		int enc1a;
		int enc1b;
		int ir0;
		int ir1;
		int us0;
		int us1;
		int accel_x;
		int accel_y;
		int accel_z;

	private:
		//Private Member Variables
		SerialSync _sync;
};

//Global Variables
std::vector<vehicle> vehicles;

//Main
int main(int argc,char* argv[])
{
	//Create Port
	std::string server_port="8080";

	//Get Command Line Port
	if(argc>1)
		server_port=argv[1];

	//Create Server
	msl::webserver server("0.0.0.0:"+server_port,service_client);
	server.setup();

	//Check Server
	if(server.good())
		std::cout<<"=)"<<std::endl;
	else
		std::cout<<"=("<<std::endl;

	//Be a server...forever...
	while(true)
	{
		//Update Rovers
		for(unsigned int ii=0;ii<vehicles.size();++ii)
			vehicles[ii].update();

		//Update Server
		server.update();

		//Give OS a Break
		usleep(0);
	}

	//Call Me Plz T_T
	return 0;
}

//Our Service Client Function Definition
bool service_client(msl::socket& client,const std::string& message)
{
	//Create Parser
	std::istringstream istr(message);

	//Parse the Request
	std::string request;
	istr>>request;
	istr>>request;

	//Vehicle Get Check
	if(msl::starts_with(request,"/get"))
	{
		//Extract ID From String
		std::string id_string=request.substr(4,request.size()-4);

		//Vehicle JSON Object
		msl::json vehicle_json;

		//Set Number of Vehicles
		vehicle_json.set("length",vehicles.size());

		//Set Vehicles
		for(unsigned int ii=0;ii<vehicles.size();++ii)
			vehicle_json.set(msl::to_string(ii),vehicles[ii].json().str());

		//Return Vehicle JSON Object
		client<<msl::http_pack_string(vehicle_json.str(),"application/json");

		//Return True (We serviced the client)
		return true;
	}

	//Vehicle Add Check
	else if(msl::starts_with(request,"/add="))
	{
		//Extract ID From String
		std::string json_string=request.substr(5,request.size()-5);

		//Temp JSON Object
		msl::json temp(json_string);

		//Extract Data
		int temp_id=msl::to_int(temp.get("id"));
		std::string temp_radio=temp.get("radio");
		int temp_baud=msl::to_int(temp.get("baud"));

		//Check for Duplicates
		bool found=false;

		for(unsigned int ii=0;ii<vehicles.size();++ii)
		{
			if(temp_id==vehicles[ii].id||temp_radio==vehicles[ii].radio)
			{
				found=true;
				break;
			}
		}

		//Add New Vehicle
		if(!found)
			vehicles.push_back(vehicle(temp_id,temp_radio,temp_baud));
	}

	//Vehicle Remove Check
	else if(msl::starts_with(request,"/rm="))
	{
		//Extract ID From String
		std::string id_string=request.substr(4,request.size()-4);

		//Convert ID to Integer
		int id=msl::to_int(id_string);

		//Find and Remove Vehicle With Passed ID
		for(unsigned int ii=0;ii<vehicles.size();++ii)
			if(id==vehicles[ii].id)
				vehicles.erase(vehicles.begin()+ii);

		//Return True (We serviced the client)
		return true;
	}

	//Vehicle Set Check
	else if(msl::starts_with(request,"/set="))
	{
		//Extract the JSON Object String
		std::string json_object_string=request.substr(5,request.size()-5);

		//Create JSON Object
		msl::json json_object(json_object_string);

		//Extract JSON Object's Data
		int json_id=msl::to_int(json_object.get("id"));
		int json_dx=msl::to_int(json_object.get("dx"));
		int json_dy=msl::to_int(json_object.get("dy"));

		//Find Vehicle With Passed ID
		for(unsigned int ii=0;ii<vehicles.size();++ii)
		{
			//If ID is Found Update Vehicle Data
			if(json_id==vehicles[ii].id)
			{
				vehicles[ii].dx=json_dx;
				vehicles[ii].dy=json_dy;
				break;
			}
		}

		//Return True (We serviced the client)
		return true;
	}

	//Vehicle Connect Check
	else if(msl::starts_with(request,"/connect="))
	{
		//Extract ID From String
		std::string id_string=request.substr(9,request.size()-9);

		//Convert ID to Integer
		int id=msl::to_int(id_string);

		//Find and Connect Vehicle With Passed ID
		for(unsigned int ii=0;ii<vehicles.size();++ii)
			if(id==vehicles[ii].id)
				vehicles[ii].connect();

		//Return True (We serviced the client)
		return true;
	}

	//Vehicle Disconnect Check
	else if(msl::starts_with(request,"/disconnect="))
	{
		//Extract ID From String
		std::string id_string=request.substr(12,request.size()-12);

		//Convert ID to Integer
		int id=msl::to_int(id_string);

		//Find and Disconnect Vehicle With Passed ID
		for(unsigned int ii=0;ii<vehicles.size();++ii)
			if(id==vehicles[ii].id)
				vehicles[ii].disconnect();

		//Return True (We serviced the client)
		return true;
	}

	//Default Return False (We did not service the client)
	return false;
}