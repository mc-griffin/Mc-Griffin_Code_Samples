// MaxisLot.cpp: implementation of the MaxisLot class.
//
//////////////////////////////////////////////////////////////////////

#include "MaxisLot.h"
#include "Utilities.h"
#include <assert.h>


MaxisLot::MaxisLot( const string &fileExt, const string &assetType )
	: MaxisAsset( fileExt, assetType )
	, _lodging(false), _food(false), _gardening(false), _shopping(false)
	, _petStore(false), _smallAnimal(false), _park(false), _spa(false) 
	, _movie(false), _model(false), _music(false)
	, _magiCo(false), _rides(false)
{
	_sectionNames.push_back( "Filters" );
}


bool MaxisLot::SetNameValue( LPCSTR name, LPCSTR value )
{
	if ( name == NULL || *name == '\0' || value == NULL || *value == '\0' )
	{
		assert(false);
		return false;
	}

	if ( _stricmp( name, "name" ) == 0 )
	{
		_name = value;
		return true;
	}
	if ( _stricmp( name, "description" ) == 0 )
	{
		_description = value;
		return true;
	}
	if ( _stricmp( name, "number" ) == 0 )
	{
		_number = value;
		return true;
	}
	if ( _stricmp( name, "floors" ) == 0 )
	{
		_floors = value;
		return true;
	}

	if ( _stricmp( name, "lodging" ) == 0 )
	{
		_lodging = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "food" ) == 0 )
	{
		_food = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "gardening" ) == 0 )
	{
		_gardening = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "shopping" ) == 0 )
	{
		_shopping = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "petstore" ) == 0 )
	{
		_petStore = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "smallanimal" ) == 0 )
	{
		_smallAnimal = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "park" ) == 0 )
	{
		_park = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "spa" ) == 0 )
	{
		_spa = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "movie" ) == 0 )
	{
		_movie = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "model" ) == 0 )
	{
		_model = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "music" ) == 0 )
	{
		_music = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "MagiCo" ) == 0 )
	{
		_magiCo = StringToBool( value );
		return true;
	}
	if ( _stricmp( name, "Rides" ) == 0 )
	{
		_rides = StringToBool( value );
		return true;
	}
	assert(false);
	return false;
}
 

bool MaxisLot::SetFeatures( const string &features )
{
	if ( features.empty()) { assert(false); return false; }

	// ensure that character is number with 5 or less digits

	DWORD len = features.length();
	if ( len == 0 || 5 < len ) { assert(false); return false; }

	for ( DWORD i = 0; i < len; ++i )
		if ( !isdigit( features[i]))
		{
			assert(false);
			return false;
		}

	_features = features;
	return true;
}


bool MaxisLot::SetGameEdition( const string &gameEdition )
{
	if ( gameEdition.empty()) { assert(false); return false; }

	// ensure that character is number with 5 or less digits

	DWORD len = gameEdition.length();
	if ( len == 0 || 5 < len ) { assert(false); return false; }

	for ( DWORD i = 0; i < len; ++i )
		if ( !isdigit( gameEdition[i]))
		{
			assert(false);
			return false;
		}

	_gameEdition = gameEdition;
	return true;
}


DWORD MaxisLot::GetNumberOfFloors() const
{
	if ( !IsSrfSelected())
		{ assert(false); return 0; }

	string floors = GetFloors();

	// floors represents a number
	for ( DWORD i = 0; i < floors.size(); ++i )
		if ( !isdigit( floors[i] ))
		{
			assert(false);
			return 0;
		}

	return atoi( floors.c_str());
}


void MaxisLot::Reset()
{
	_name.empty();
	_description.empty();
	_number.empty();
	_floors.empty();
	_lodging = false;
	_food = false;
	_gardening = false;
	_shopping = false;
	_petStore = false;
	_smallAnimal = false;
	_park = false;
	_spa = false;
	_movie = false;
	_model = false;
	_music = false;
}


string MaxisLot::GetXmlDoc() const
{
	string x = "<?xml version=\"1.0\"?>\r\n\r\n";
	x += "<" + GetAssetType() + ">\r\n";

	x += "<Name>";
	x += GetXmlEncoded( GetName());
	x += "</Name>\r\n";

	x += "<Description>";
	x += GetXmlEncoded( GetDescription());
	x += "</Description>\r\n";

	x += "<Number>";
	x += GetXmlEncoded( GetNumber());
	x += "</Number>\r\n";

	x += "<Floors>";
	x += GetXmlEncoded( GetFloors());
	x += "</Floors>\r\n";

	x += "<Lodging>";
	x += GetLodging() ? "true" : "false";
	x += "</Lodging>\r\n";

	x += "<Food>";
	x += GetFood() ? "true" : "false";
	x += "</Food>\r\n";

	x += "<Gardening>";
	x += GetGardening() ? "true" : "false";
	x += "</Gardening>\r\n";

	x += "<Shopping>";
	x += GetShopping() ? "true" : "false";
	x += "</Shopping>\r\n";

	x += "<PetStore>";
	x += GetPetStore() ? "true" : "false";
	x += "</PetStore>\r\n";

	x += "<SmallAnimal>";
	x += GetSmallAnimal() ? "true" : "false";
	x += "</SmallAnimal>\r\n";

	x += "<Park>";
	x += GetPark() ? "true" : "false";
	x += "</Park>\r\n";

	x += "<Spa>";
	x += GetSpa() ? "true" : "false";
	x += "</Spa>\r\n";

	x += "<Movie>";
	x += GetMovie() ? "true" : "false";
	x += "</Movie>\r\n";

	x += "<Model>";
	x += GetModel() ? "true" : "false";
	x += "</Model>\r\n";

	x += "<Music>";
	x += GetMusic() ? "true" : "false";
	x += "</Music>\r\n";

	x += "<MagiCo>";
	x += GetMagiCo() ? "true" : "false";
	x += "</MagiCo>\r\n";

	x += "<Rides>";
	x += GetRides() ? "true" : "false";
	x += "</Rides>\r\n";

	x += "<Type>"; // meaning lot type, not asset type
	x += GetXmlEncoded( GetFeatures());
	x += "</Type>\r\n";

	x += "<Game_Edition>";
	x += GetXmlEncoded( GetGameEdition());
	x += "</Game_Edition>\r\n";

	x += "</" + GetAssetType() + ">\r\n";

	return x;
}

