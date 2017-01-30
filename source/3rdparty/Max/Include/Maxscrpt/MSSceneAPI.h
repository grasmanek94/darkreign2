//***************************************************************************
//* SceneAPI - Implementation of Scene Extension API for 3D Studio MAX 1.2
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* November 2, 1996	CCJ Initial coding
//* January  8, 1997	CCJ Added material editor slot access
//* March   15, 1997	CCJ Added scene materials access
//*
//* This class implements a couple of missing API calls.
//* 
//* WARNING:
//* These functions depend on the internal structure of 3D Studio MAX 1.2.
//* Do not attempt to use it with other versions.
//*

// If you encounter this error, you are using this file with an unsupported
// version of the 3D Studio Max SDK.
//#if	VERSION_3DSMAX != 120
//#error "STOP!!! This file is only for use with 3D Studio MAX Version 1.2"
//#endif

class SceneAPI {
public:
	SceneAPI(Interface* i);

	int GetNumAtmos();
	Atmospheric* GetAtmospheric(int i);
	Control* GetAmbientLightController();
	Color GetAmbientLight(TimeValue t);
	Texmap* GetBackgroundEnvironment();
	Control* GetBackgroundColorController();
	Color GetBackgroundColor(TimeValue t);
	MtlBase* GetMtlSlot(int i);
	void SetMtlSlot(int i, MtlBase* m);
	int NumMtlSlots();
	MtlBaseLib* GetSceneMtls();

private:
	void FindScene();
	void FindRenderEnvironment();

	Interface* ip;
	ReferenceMaker* scene;
	ReferenceMaker* rendEnv;
};
