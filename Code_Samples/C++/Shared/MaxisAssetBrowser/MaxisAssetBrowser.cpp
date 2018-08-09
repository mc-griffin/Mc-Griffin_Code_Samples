// MaxisAssetBrowser.cpp: implementation of the MaxisAssetBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "MaxisAssetBrowser.h"
#include "FileUtils.h"


// To accomodate a LNK2001 error obliquely described in Q239436
// (PRB: LNK2001 on Template Member Functions)
// this class must instantiate itself with every asset.
#include "HdLot.h"
// added HdLot and ..\MaxisAsset to included directories in dsp
template class MaxisAssetBrowser<HdLot>;
// since HdNpc uses Packager, I must include it in this build
#include "HdNpc.h"
template class MaxisAssetBrowser<HdNpc>;
#include "GolfCourse.h"
template class MaxisAssetBrowser<GolfCourse>;
#include "VacationLot.h"
template class MaxisAssetBrowser<VacationLot>;
#include "SuperstarStudio.h"
template class MaxisAssetBrowser<SuperstarStudio>;
#include "SuperstarNpc.h"
template class MaxisAssetBrowser<SuperstarNpc>;
#include "MakinMagicLot.h"
template class MaxisAssetBrowser<MakinMagicLot>;

template <class T>
bool MaxisAssetBrowser<T>::SelectDir( const string &dirIn )
{
	T asset;
	if ( asset._fileExt.empty()) { assert(false); return false; }

	string dir = MaxisFileBrowser::MakeWinPath( dirIn );
	if ( dir.empty()) { assert(false); return false; }

	_assets.MakeEmpty();

	bool scan = _browser.SelectDir( dir, asset._fileExt );
	if ( !scan ) return false;

	string file = _browser.GetFirstFile();
	while ( !file.empty())
	{
		string srfPath = MaxisFileBrowser::ConvertFileNameToSrf( dir + "\\" + file );
		if ( !srfPath.empty() && MaxisFileBrowser::FileExists( srfPath ))
		{
			bool set = asset.SelectSrf( srfPath );
			if ( set )
			{
				if ( asset.AllNeededFilesExist())
				{
					string fileNameSansExt;
					if ( SplitFilePath( srfPath, NULL, &fileNameSansExt, NULL ))
					{
						string srfFileName = fileNameSansExt + ".srf";
						_assets.Append( srfFileName );
					}
					else { assert(false); }
				}
			}
		}
		else { assert(!srfPath.empty()); }

		file = _browser.GetNextFile();	
	}
	return true;
}


template <class T>
string MaxisAssetBrowser<T>::GetFirstDir()
{
	return _browser.GetFirstDir();
}


template <class T>
string MaxisAssetBrowser<T>::GetNextDir()
{
	return _browser.GetNextDir();
}


template <class T>
string MaxisAssetBrowser<T>::GetFirstFile()
{
	bool select = _assets.SelectFirst();
	if ( !select ) return "";

	return _assets.GetSelected();
}


template <class T>
string MaxisAssetBrowser<T>::GetNextFile()
{
	bool select = _assets.SelectNext();
	if ( !select )
	{
		assert( 0 < _assets.GetSize());
		return "";
	}

	return _assets.GetSelected();
}


template <class T>
DWORD MaxisAssetBrowser<T>::GetDirCount() const
{
	return _browser.GetDirCount();
}


template <class T>
DWORD MaxisAssetBrowser<T>::GetFileCount() const
{
	return _assets.GetSize();
}

