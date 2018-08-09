// MaxisLot.h: interface for the MaxisLot class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAXISLOT_H__D8D00CBB_57D9_4C32_B928_045CA8E8AC93__INCLUDED_)
#define AFX_MAXISLOT_H__D8D00CBB_57D9_4C32_B928_045CA8E8AC93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MaxisAsset.h"
#include "XmlDoc.h"


class MaxisLot : public MaxisAsset, public XmlDoc
{
protected:
	// As of Unleashed, lots can have 2 sections:
	// 1. either CommunityLot, ResidentialLot or something else
	// 2. Filters
	// As of Superstar, there are new filters: spa, movie, model, music.
	// Further, Superstar lots should not have pet stores.

	// lot info from .srf
	string _name;
	string _description;
	string _number;
	string _floors;

	// [Filters]
	bool _lodging;
	bool _food;
	bool _gardening;
	bool _shopping;
	bool _petStore;
	bool _smallAnimal;
	bool _park;
	// Superstar
	bool _spa;
	bool _movie;
	bool _model;
	bool _music;
	// Makin' Magic
	bool _magiCo; 
	bool _rides;

	// _features and _gameEdition are specified on website by user when a HotDate
	// or Vacation lot is uploaded, but not an Unleashed or Superstar lot.
	string _features;
	string _gameEdition;

public:
	MaxisLot( const string &fileExt, const string &assetType );

	virtual bool AllNeededFilesExist() const = 0;

	virtual string GetXmlDoc() const;

	string GetName() const { return _name; }
	string GetDescription() const { return _description; }
	string GetNumber() const { return _number; }
	string GetFloors() const { return _floors; }
	DWORD GetNumberOfFloors() const;

	bool GetLodging() const { return _lodging; }
	bool GetFood() const { return _food; }
	bool GetGardening() const { return _gardening; }
	bool GetShopping() const { return _shopping; }
	bool GetPetStore() const { return _petStore; }
	bool GetSmallAnimal() const { return _smallAnimal; }
	bool GetPark() const { return _park; }
	bool GetSpa() const { return _spa; }
	bool GetMovie() const { return _movie; }
	bool GetModel() const { return _model; }
	bool GetMusic() const { return _music; }
	bool GetMagiCo() const { return _magiCo; }
	bool GetRides() const { return _rides; }

	bool SetFeatures( const string &features );
	string GetFeatures() const { return _features; }
	bool SetGameEdition( const string &gameEdition );
	string GetGameEdition() const { return _gameEdition; }

	void Reset();

protected:
	virtual bool SetNameValue( LPCSTR name, LPCSTR value );

	// hide MaxisAsset::SelectIniFile(), forcing clients to
	// use SelectSrf()
	bool SelectIniFile( const string &filePath );
};


#endif // !defined(AFX_MAXISLOT_H__D8D00CBB_57D9_4C32_B928_045CA8E8AC93__INCLUDED_)
