// party.cpp

#include "../util/typedefs.h"
#include "../util/config.h"
#include "gmw.h"
#include "party.h"
#include <sstream>

#include<ctime>
#include<sys/time.h>

#include<fstream>

using namespace std;

#ifdef _DEBUG
#include <cassert>
using namespace std;
#endif


#define AsOTSender(nPID1, nPID2)	\
	( (nPID1<nPID2 && ((nPID2-nPID1)%2==1)) || (nPID1>nPID2 && ((nPID1-nPID2)%2 == 0)))
 
//#define AsOTSender(nPID1, nPID2)	 (nPID1<nPID2)

CParty::CParty()
{
    GetInstance(this);
    m_pCircuit = NULL;        
}


CParty::~CParty()
{
	Cleanup();
}

BOOL CParty::Init()
{
	// circuits
        CConfig* cf = CConfig::GetInstance();
	
	if( !cf->GetCircFileName().empty() )
	{
	    //Aseem: Binary loading of circuit
	    //m_pCircuit = LOAD_CIRCUIT_HEADER_BIN(cf->GetCircFileName().c_str());
	  m_pCircuit = LOAD_CIRCUIT_TXT(cf->GetCircFileName().c_str());

		if(!m_pCircuit)
		{
			cout << "failure in loading circuit " << cf->GetCircFileName() << endl;
			return FALSE;
		}
	}
	else
	{
	    //m_pCircuit = CREATE_CIRCUIT(m_nNumParties, cf->GetCircCreateName(), cf->GetCircCreateParams());
	    cout << "Create circuit not supported" << endl;
	}
	
	/*if(cf->GetDoOTNum() == 0) {
	    m_nANDGates = m_pCircuit->GetNumANDs();
	    } else {*/
	//m_nANDGates = cf->GetDoOTNum();
	//cout << "setting DO OT Num to " << m_nANDGates << endl;
	//}
	//m_nNumParties = cf->GetNumParties();
	//m_nPID = cf->GetPID();

	// // for Naor-Pinkas
	// m_p = CConfig::GetInstance()->GetPrime();
	// m_q = m_p/2 -1;
	// conv(m_g, CConfig::GetInstance()->GetGenerator());
	
	// // socket	
	// //m_vSockets.resize(m_nNumParties);

	// // for sha-random
	// sha1_context sha;
	// sha1_starts(&sha);
	// sha1_update(&sha, (BYTE*) cf->GetSeed().c_str(), cf->GetSeed().length());
	// sha1_finish(&sha, m_aSeed);

	// m_vCounter.resize(m_nNumParties);
	// for(int i=0; i<m_nNumParties; i++)
	// {
	// 	m_vCounter[i] = 0;
	// }
	
	// // for OT Preprocessing
	// m_vWWIn.resize(m_nNumParties);
	// m_vWWOut.resize(m_nNumParties);
	// m_vGMWRnd.resize(m_nNumParties);
	// for(int i=0; i<m_nNumParties; i++)
	//     {
	// 	if( i == m_nNumParties ) 
	// 		continue;

	// 	if( AsOTSender(m_nPID, i) )
	// 	{
	// 		// Play as Sender
	// 		m_vWWIn[i].Create(m_nANDGates*4, m_aSeed, i, m_vCounter[i]);	
	// 		m_vGMWRnd[i].Create(m_nANDGates, m_aSeed, i, m_vCounter[i]);
	// 	}
	// 	else
	// 	{
	// 		// Play as Receiver
	// 	    m_vWWIn[i].Create(m_nANDGates*2, m_aSeed, i, m_vCounter[i]);	
	// 		m_vWWOut[i].Create(m_nANDGates);
	// 		m_vGMWRnd[i].Create(m_nANDGates);
	// 	}
	// }

	// for private key setup
#ifdef IMPL_PRIVIATE_CHANNEL
	m_vPKeys.resize(m_nNumParties);
	for(int i=0; i<m_nNumParties; i++)
	{
		if( i == m_nPID ) 
			continue;

		if( AsOTSender(m_nPID, i) )
		{
			// Sender is N-P sender
			m_vPKeys[i].Create(SHA1_BITS);
		}
		else
		{
			// Receiver is N-P sender
			m_vPKeys[i].Create(SHA1_BITS, m_aSeed, i, m_vCounter[i]);	
		}
		 
	}
#endif //IMPL_PRIVIATE_CHANNEL

	// thread creation	
	/*m_vThreads.resize(m_nNumParties);
	for(int i=0; i<m_nNumParties; i++ )
	{
		if( i != m_nPID )
		{
			m_vThreads[i] = new CWorkerThread(i);
			m_vThreads[i]->Start();
		}
		}*/
	
	//TODO: Aseem ??
        ThreadRunCircuit();

	return TRUE;
}

/*void CParty::DumpOTStuff()
{
    //m_vGMWRnd, m_vWWIn, m_vWWOut
    //these are vector of CBitVector, indexed by parties

    CConfig *cf = CConfig::GetInstance();
    ofstream outfile;
    ofstream outfilemeta;
    
    outfile.open(cf->GetOTFileName().c_str(), ios::out | ios::trunc);
    outfilemeta.open(cf->GetOTMetaFileName().c_str(), ios::out | ios::trunc);
    
    //dump m_vGMWRnd
    for(int i = 0; i < m_nNumParties; ++i) {
	CBitVector& vec = m_vGMWRnd[i];
	//dump this CBitVector
	outfilemeta << vec.GetSize() << " " << vec.GetRand() << " ";
	for(int j = 0; j < vec.GetSize(); ++j) {
	    outfile << vec.GetByte(j);
	}
    }

    //dump m_vWWIn
    for(int i = 0; i < m_nNumParties; ++i) {
	CBitVector& vec = m_vWWIn[i];
	//dump this CBitVector
	outfilemeta << vec.GetSize() << " " << vec.GetRand() << " ";
	for(int j = 0; j < vec.GetSize(); ++j) {
	    outfile << vec.GetByte(j);
	}
    }

    //dump m_vWWOut
    for(int i = 0; i < m_nNumParties; ++i) {
	CBitVector& vec = m_vWWOut[i];
	//dump this CBitVector
	outfilemeta << vec.GetSize() << " " << vec.GetRand() << " ";
	for(int j = 0; j < vec.GetSize(); ++j) {
	    outfile << vec.GetByte(j);
	}	
    }

    outfile.close();
    outfilemeta.close();
}

void CParty::LoadOTStuff()
{
    cout << "Loading OT Stuff" << endl;
    CConfig *cf = CConfig::GetInstance();
    ifstream infile(cf->GetOTFileName ().c_str());
    ifstream infilemeta(cf->GetOTMetaFileName ().c_str());
    
    cout << "OT file name: " << cf->GetOTFileName() << endl;
    m_vGMWRnd.clear();
    m_vWWIn.clear();
    m_vWWOut.clear();
    
    m_vGMWRnd.resize(m_nNumParties);
    m_vWWIn.resize(m_nNumParties);
    m_vWWOut.resize(m_nNumParties);


    //load m_vGMWRnd
    for(int i = 0; i < m_nNumParties; ++i) {
	int size;
	BYTE* bits;
	int rnd;
	BYTE b;
	
	infilemeta >> size;
	infilemeta >> rnd;
	bits = new BYTE[size];
	for(int j = 0; j < size; ++j) {
	    if(infile.good()) {
		b = (unsigned char) infile.get();
		bits[j] = b;
	    } else {
		exit(1);
	    }
	}
	m_vGMWRnd[i].AttachAll(size, bits, rnd);
    }
    //load m_vWWIn
    for(int i = 0; i < m_nNumParties; ++i) {
	int size;
	BYTE* bits;
	int rnd;
	BYTE b;

	infilemeta >> size;
	infilemeta >> rnd;
	bits = new BYTE[size];
	for(int j = 0; j < size; ++j) {	    
	    if(infile.good()) {
		b = (unsigned char) infile.get();
		bits[j] = b;
	    } else {
		exit(1);
	    }
	}
	m_vWWIn[i].AttachAll(size, bits, rnd);
    }
    //load m_vWWOut
    for(int i = 0; i < m_nNumParties; ++i) {
	int size;
	BYTE* bits;
	int rnd;
	BYTE b;

	infilemeta >> size;
	infilemeta >> rnd;
	bits = new BYTE[size];
	for(int j = 0; j < size; ++j) {
	    if(infile.good()) {
		b = (unsigned char) infile.get();
		bits[j] = b;
	    } else {
		if(infile.eof()) { cout << "EOF" << endl; }
		if(infile.bad()) { cout << "BAD" << endl; }
		if(infile.fail()) { cout << "FAIL" << endl; }
		cout << "infile not good" << endl << flush << endl;
		exit(1);
	    }
	}
	m_vWWOut[i].AttachAll(size, bits, rnd);
    }

    infile.close();
    infilemeta.close();
    cout << "Loading OT stuff done" << endl;
    }*/

void CParty::Cleanup()
{
	if( m_pCircuit ) delete m_pCircuit;

	for(int i=0; i<m_nNumParties; i++ )
	{
		if( i != m_nPID )
		{
			m_vThreads[i]->PutJob(e_Stop);
			m_vThreads[i]->Wait();
			delete m_vThreads[i];
		} 
	}
}

BOOL CParty::ThreadRunCircuit()
{
	CConfig* cf = CConfig::GetInstance();

	ostringstream os;

	string strFileName = cf->GetCircFileName();
	if( !strFileName.empty() )
	{
	  //os << "loading circuit " << strFileName << endl;
	  //cout << os.str() << flush;
	  //os << "m_pCircuit = " << m_pCircuit << endl << flush;
		//Aseem: Binary loading of circuit ?
		//m_pCircuit->LoadBin(cf->GetCircFileName().c_str());
	    //m_pCircuit->Load(cf->GetCircFileName().c_str());
		//os << "done loading" << endl;
	}

	m_pGates = m_pCircuit->Gates();
	
	// initialize gates
	m_pGates[0].val = 0;			// constant 0

	// XOR should be 1
	if( m_nPID == m_nNumParties - 1 )
		m_pGates[1].val = 1;			// constant 1
	else
		m_pGates[1].val = 0;			// constant 0

	// set initial default values 
	for(int i=2; i<m_pCircuit->GetNumGates(); i++)
	{
		m_pGates[i].val = GATE_INITIALIZED;
	}

	int o_start = m_pCircuit->GetOutputStart(m_nPID);
	int o_end = m_pCircuit->GetOutputEnd(m_nPID);
	if( o_start <= o_end )
		m_Outputs.Create(o_end-o_start+1);
	
	//os.str("");
	//os << " circuit initialized..." << endl;
	//cout << os.str() << flush;

	return TRUE;
 }

int CParty::Run(struct perf_data& p)
{
    int err = 1;
    struct timeval initial;
    struct timeval doneload;
    struct timeval doneinput;
    struct timeval donegates;
    struct timeval doneoutput;

    gettimeofday(&initial, NULL);
    //cout <<"Initialization..." << endl;
    if(!Init())
	return err;

    gettimeofday(&doneload, NULL);
    //CCircuitThread* pThread = new CCircuitThread();
    //pThread->Start();
    
    // socket stuff
    /*cout << "Establishing Connection..." << endl;
    if(!EstablishConnection())
    return err;*/
    
    // setup ot extension

    // cout << "Setting up OT Extension..." << endl;
    // if(CConfig::GetInstance()->GetDoOT()) {
    // 	if(!SetupOTExtension())
    // 	    return err;
    // 	//DumpOTStuff();
    // } else {
    // 	//LoadOTStuff();
    // }
    
    //cout << "Waiting on CCiruitThread ..." << endl;
    //pThread->Wait();
    //delete pThread;
	
    //cout << "Sharing Input..." << endl;
    // input sharing
    if(!ShareInput())
	return err;
    
    //cout << "Loading Shares ..." << endl;
    if(!LoadShares())
	return err;
 
    gettimeofday(&doneinput, NULL);
    //cout << "Computing Gates..." << endl;
    // multiplication
    if(!ComputeMultiplications())
	return err;

    gettimeofday(&donegates, NULL);
    //cout << "Computing Output..." << endl;
    // output reconstruction
    ComputeOutput();

    gettimeofday(&doneoutput, NULL);

    long int initialu = initial.tv_sec * 1000000 + initial.tv_usec;
    long int doneloadu = doneload.tv_sec * 1000000 + doneload.tv_usec;
    long int doneinputu = doneinput.tv_sec * 1000000 + doneinput.tv_usec;
    long int donegatesu = donegates.tv_sec*1000000 + donegates.tv_usec;
    long int doneoutputu = doneoutput.tv_sec*1000000 + doneoutput.tv_usec;

    // cout << "time to load circuit: " << doneloadu - initialu << endl;
    // cout << "time to send input shares: " << doneinputu - doneloadu << endl;
    // cout << "time to compute gates: " << donegatesu - doneinputu << endl;
    // cout << "time to set up output: " << doneoutputu - donegatesu << endl;

    p.load = doneloadu - initialu;
    p.input_setup = doneinputu - doneloadu;
    p.compute_gates = donegatesu - doneinputu;
    p.output_setup = doneoutputu - donegatesu;

    return 0;
}


//=========================================================
// Connecting Routines
BOOL CParty::EstablishConnection(vector<string> &addrs, vector<USHORT> &ports,
				 int num_parties, int self_id, int and_gates)
{
    m_Addrs = addrs;
    m_Ports = ports;
    
    m_nNumParties = num_parties;
    m_nPID = self_id;

    m_nANDGates = and_gates;

    // socket	
    m_vSockets.resize(m_nNumParties);

    m_vThreads.resize(m_nNumParties);
    for(int i=0; i<m_nNumParties; i++ )
	{
	    if( i != m_nPID )
		{
		    m_vThreads[i] = new CWorkerThread(i);
		    m_vThreads[i]->Start();
		}
	}

    
    WakeupWorkerThreads(e_Connect);
	
    BOOL bSuccess = MainThreadRunListen();
    BOOL bSuccess2 = WaitWorkerThreads();

    BOOL allok = bSuccess & bSuccess2;
    
    if(!allok) { return allok; }

    // for Naor-Pinkas

    stringstream p_ss;
    p_ss << "8936097950764538541647693880373941060412422053330581106416547022143872696986309618392492140173286901378451169263228782167065246660381840799235731486130087";
    
    p_ss >> m_p;
    m_q = m_p/2 -1;
    
    ZZ g;
    
    stringstream g_ss;
    g_ss << "7660915846360464914746169975675514711978996378800371841846530440188167304050642048045944447898172503094027848348928324282770185052123039167705188057761352";
    g_ss >> g;

    conv(m_g, g);


    string seed = "11323434087201";
    sha1_context sha;
    sha1_starts(&sha);
    sha1_update(&sha, (BYTE*) seed.c_str(), seed.length());
    sha1_finish(&sha, m_aSeed);


    m_vCounter.resize(m_nNumParties);
    for(int i=0; i<m_nNumParties; i++)
	{
	    m_vCounter[i] = 0;
	}	

    // for OT Preprocessing
    m_vWWIn.resize(m_nNumParties);
    m_vWWOut.resize(m_nNumParties);
    m_vGMWRnd.resize(m_nNumParties);
    for(int i=0; i<m_nNumParties; i++)
	{
	    if( i == m_nNumParties ) 
		continue;

	    if( AsOTSender(m_nPID, i) )
		{
		    // Play as Sender
		    m_vWWIn[i].Create(m_nANDGates*4, m_aSeed, i, m_vCounter[i]);	
		    m_vGMWRnd[i].Create(m_nANDGates, m_aSeed, i, m_vCounter[i]);
		}
	    else
		{
		    // Play as Receiver
		    m_vWWIn[i].Create(m_nANDGates*2, m_aSeed, i, m_vCounter[i]);	
		    m_vWWOut[i].Create(m_nANDGates);
		    m_vGMWRnd[i].Create(m_nANDGates);
		}
	}
    
    //cout << "Setting up OT extension" << endl;
    if(!SetupOTExtension()) {
	return false;
    }
    //cout << "Done with OT extension" << endl;

    m_nMultIdx = 0;

    return true;
}

BOOL CParty::ThreadRunConnect(int nID)    
{
	// Connect to parties with lower pid
	if( nID >= m_nPID ) return TRUE;

	BOOL bFail = FALSE;
	LONG lTO = CONNECT_TIMEO_MILISEC;
	CConfig* cf = CConfig::GetInstance();

	ostringstream os;

	//cout << "Connecting party "<< nID <<": " << m_Addrs[nID] << ", " << m_Ports[nID] << endl;

	for( int i=0; i<RETRY_CONNECT; i++ )
	{
		if( !m_vSockets[nID].Socket() ) 
			goto connect_failure; 

		if( m_vSockets[nID].Connect( m_Addrs[nID], m_Ports[nID], lTO))
		{
			// send pid when connected
			m_vSockets[nID].Send( &m_nPID, sizeof(int) );

			/*os.str("");
			os << " (" << nID << ") connected" << endl;
			cout << os.str() << flush;*/

			return TRUE;
		}

		m_vSockets[nID].Close();
	}

connect_failure:

	os.str("");
	os << " (" << nID << ") connection failed" << endl;
	cout << os.str() << flush;
	return FALSE;
}

BOOL CParty::MainThreadRunListen()
{
	// listen to parties with higher pid
	if( m_nPID == m_nNumParties - 1 ) return TRUE;

	ostringstream os;
	/*CConfig* cf = CConfig::GetInstance();*/
	//cout << "Listening: " << m_Addrs[m_nPID] << ":" << m_Ports[m_nPID] << endl;


	if( !m_vSockets[m_nPID].Socket() ) 
		goto listen_failure;

	if( !m_vSockets[m_nPID].Bind(m_Ports[m_nPID], m_Addrs[m_nPID]) )
		goto listen_failure;

	if( !m_vSockets[m_nPID].Listen() )
		goto listen_failure;

	for( int i = m_nPID + 1; i<m_nNumParties; i++ )
	{
    	        CSocket sock;
		if( !m_vSockets[m_nPID].Accept(sock) )
			goto listen_failure;

		// receive initial pid when connected
		UINT nID;
		sock.Receive(&nID, sizeof(int));

		if( nID >= UINT(m_nNumParties) )  
		{
			//cout << "erroneous party " << nID <<": ignoring... " << endl;
			sock.Close();
			i--;
			continue;
		}

		/*os.str("");
		os <<  " (" << nID <<") connection accepted" << endl;
		cout << os.str() << flush;*/

		// locate the socket appropriately
		m_vSockets[nID].AttachFrom(sock);
		sock.Detach();
	}

	//cout << "Listening finished"  << endl;

	return TRUE;

listen_failure:
	cout << "Listen failed" << endl;
	return FALSE;
}



//=============================================================== 
// input sharing routines
BOOL CParty::ShareInput()
{
	WakeupWorkerThreads(e_ShareInput);
	BOOL bSuccess = MainThreadRunSendInputShare();
	BOOL bSuccess2 = WaitWorkerThreads();
	return bSuccess & bSuccess2;
}

BOOL CParty::LoadShares()
{
    int shStart = m_pCircuit->GetShInputStart();
    int shEnd   = m_pCircuit->GetShInputEnd();

    if(shStart != 0 && shEnd != 0) {
	// load some shares
	vector<char> vIn;
	CConfig::GetInstance()->GetShInput(vIn);
	
	for(int i = shStart; i <= shEnd; ++i) {
	    char val = vIn[i - shStart];
	    m_pGates[i].val = val;
	}
    }
    
    return true;
}

void CParty::GetSharesOutput(vector<char>& shares)
{
    int shStart = m_pCircuit->GetShOutputStart();
    int shEnd   = m_pCircuit->GetShOutputEnd();

    if(shStart != 0 && shEnd != 0) {
	// load some shares
	for(int i = shStart; i <= shEnd; ++i) {
	    shares.push_back(m_pGates[i].val);
	}
    }
}

BOOL CParty::MainThreadRunSendInputShare()
{
	int nStart = m_pCircuit->GetInputStart(m_nPID);
	int nEnd = m_pCircuit->GetInputEnd(m_nPID);
	int nSize = nEnd - nStart + 1;

	ostringstream os;
	//os << " (" << m_nPID << ") setting input gate from " << nStart << " to " << nEnd << endl;
	//cout << os.str() << flush;

	CConfig* cf = CConfig::GetInstance();
	int nInputs = cf->GetNumInputs();
	
	vector<CBitVector> vShares;
	vShares.resize(m_nNumParties);
	for(int i=0; i<m_nNumParties; i++ )
	{
		vShares[i].Create(nSize, m_aSeed, i, m_vCounter[i]);  
	}

	// bind the input
	int nBits = m_pCircuit->GetNumVBits(m_nPID); 
	 
	#ifdef _DEBUG
	assert( nSize == nBits * nInputs );
	#endif

	vector<int> vIn;
	CConfig::GetInstance()->GetInput(vIn);	

		
	if( vIn.size() < UINT(nInputs) )
	{
	    cout << " error: wrong input" << endl;
	    return FALSE;
	}
	
	int j=nStart;
	int mask; 
	int val;
	for( int i=0; i<nInputs && j<=nEnd; i++ ) 
	{
		for(int k=0; k<nBits && j<=nEnd; k++ )
		{
			mask = (1 << k);
			val = !!(vIn[i] & mask);
			
			for(int u=0; u<m_nNumParties; u++)
			{
				if( u != m_nPID )
					val ^= vShares[u].GetBit(i*nBits+k);
			}

			m_pGates[j++].val = val; 
		}
	}
 
	int nSizeBytes = (nSize+7)/8;
	for( int i=0; i<m_nNumParties; i++ )
	{
		if( i != m_nPID )
		{
			m_vSockets[i].Send( vShares[i].GetArr(), nSizeBytes);

			//os.str("");
			//os << " (" << i << ") sent input share: " << nSizeBytes  << " bytes"<< endl;
			//cout << os.str() << flush;
		}
	}

	return TRUE;
}


BOOL CParty::ThreadRunRecvInputShare(int nSndID)
{
	CBitVector share; 

	int nStart = m_pCircuit->GetInputStart(nSndID);
	int nEnd = m_pCircuit->GetInputEnd(nSndID);
	int nSize = nEnd - nStart + 1;
	int nTotalBytes = (nSize+7)/8;

	// bind the input
	share.Create(nSize);
	m_vSockets[nSndID].Receive(share.GetArr(), nTotalBytes);
		
	for( int i=nStart,j=0; i<=nEnd; i++,j++ ) 
	{
	    m_pGates[i].val = share.GetBit(j);
	}

	ostringstream os;
	//os << " (" << nSndID << ") received input share: " << nStart << " to " << nEnd << endl;
	//cout << os.str() << flush;

 
	return TRUE;
}


//=================================================================
// OT Extension Routines
BOOL CParty::SetupOTExtension()
{
	// IKNP-first step: receiver of Naor-Pinkas
        m_vIKNPT.resize(m_nNumParties);
	m_vIKNPU.resize(m_nNumParties);
	m_vIKNPMtx.resize(m_nNumParties);
	m_vIKNPProgress.resize(m_nNumParties);
	for(int i=0; i<m_nNumParties; i++)
		m_vIKNPProgress[i] = 0;
	
	WakeupWorkerThreads(e_OTExt);

	vector<CThread*> vThreads;
	for(int i=0; i<m_nNumParties; i++)
	{
		if( i == m_nPID ) continue;

		CThread* pThread = new COTHelperThread(i);
		pThread->Start();
		vThreads.push_back( pThread );
	}
	
	while(!vThreads.empty())
	{
	        vThreads.back()->Wait();
		delete vThreads.back();
		vThreads.pop_back();
	}

	return WaitWorkerThreads();
}

BOOL CParty::ThreadRunIKNPSndFirst(int nRcvID)
{
	//=================================================
	// N-P receiver: receive: C0 (=g^r), C1, C2, C3 
	CSocket& sock = m_vSockets[nRcvID];
	int nFieldSize = NumBytes(m_p);

	// will be used later also: allocate sufficient amount
	BYTE* pBuf = new BYTE[NUM_EXECS_NAOR_PINKAS*nFieldSize];	
	
	int nBufSize = 4 * nFieldSize;	
	sock.Receive(pBuf, nBufSize);
	
	BYTE* pBufIdx = pBuf;
	ZZ* pC = new ZZ[4];
	for(int i=0; i<4; i++)
	{
		ZZFromBytes(pC[i], pBufIdx, nFieldSize);
		pBufIdx += nFieldSize;
	}
	
	//====================================================
	// N-P receiver: send pk0 
	ZZ PK_sigma, PK0;   
	ZZ* pK =  new ZZ[NUM_EXECS_NAOR_PINKAS];

	CBitVector& U = m_vIKNPU[nRcvID];
	U.Create(NUM_EXECS_NAOR_PINKAS*2, m_aSeed, nRcvID, m_vCounter[nRcvID]);  
	pBufIdx = pBuf;

	CBitVector rnd;
	rnd.Create(NUM_EXECS_NAOR_PINKAS*nFieldSize*8, m_aSeed, nRcvID, m_vCounter[nRcvID]);
	BYTE* pBufRnd = rnd.GetArr();

	int choice;
	ZZ ztmp;
	for(int i=0; i<NUM_EXECS_NAOR_PINKAS; i++)
	{
		// random k
		ZZFromBytes(ztmp, pBufRnd, nFieldSize);
		rem(pK[i], ztmp, m_q);
		pBufRnd += nFieldSize;

		// pk_sig = g^k
		PowerMod(PK_sigma, m_g, pK[i], m_p);
		 
		// pk0 = C_sig/pk_sig
		choice = U.Get2Bits(i);
		if( choice != 0 )
		{
			InvMod(ztmp, PK_sigma, m_p);
			MulMod(PK0, pC[choice], ztmp, m_p);
		}
		else
		{
			PK0 = PK_sigma;
		}
		
		// put pk0
		BytesFromZZ(pBufIdx, PK0, nFieldSize);
		pBufIdx += nFieldSize;
	}
 
	sock.Send(pBuf, NUM_EXECS_NAOR_PINKAS * nFieldSize);
	//cout << " (" << nRcvID << ") sent PK0 " << endl;

	//ostringstream os;
	// compute g^{rk}
	
	vector<sha1_context> vKeySeeds(NUM_EXECS_NAOR_PINKAS);
	ZZ* pDec = new ZZ[NUM_EXECS_NAOR_PINKAS];
	for(int i=0; i<NUM_EXECS_NAOR_PINKAS; i++ )
	{
	 	PowerMod(pDec[i], pC[0], pK[i], m_p);
		BytesFromZZ(pBuf, pDec[i], nFieldSize);
		
		sha1_context& sha = vKeySeeds[i]; 
		sha1_starts(&sha);
		sha1_update(&sha, pBuf, nFieldSize);
 	} 

	delete [] pK;
	delete [] pC;
	
	//cout << " (" << nRcvID << ")  computed decryption keys " << endl;

	// NP receiver: receive Enc(M0), Enc(M1), Enc(M2), Enc(M3)
#ifdef IMPL_PRIVIATE_CHANNEL
	{
		sock.Receive(pBuf, SHA1_BYTES*4);
  		int index = PKEY_SHA_INDEX;
		choice = U.Get2Bits(0);
		sha1_context sha2 = vKeySeeds[0];
		sha1_update(&sha2, (BYTE*) &index, sizeof(int));
		sha1_finish(&sha2, m_vPKeys[nRcvID].GetArr());
		m_vPKeys[nRcvID].XOR(pBuf + SHA1_BYTES*choice, SHA1_BYTES);
	}
#endif //IMPL_PRIVIATE_CHANNEL	
		 
	delete [] pBuf;
	CBitMatrix& matrix = m_vIKNPMtx[nRcvID]; 
	matrix.Create(NUM_EXECS_NAOR_PINKAS, m_nANDGates);
	
	// NP receiver: compute M_sigma
	int nWindowBytes = NUM_EXECS_NAOR_PINKAS*4*OT_WINDOW_SIZE_BYTES;
	BYTE* pBuf2 = new BYTE[NUM_EXECS_NAOR_PINKAS*4*OT_WINDOW_SIZE_BYTES];

 	int& nProgress = m_vIKNPProgress[nRcvID]; 
	int nToRcvBytes = (NUM_EXECS_NAOR_PINKAS*4*m_nANDGates + 7)/8;
	vector<CBitVector> vKeys(NUM_EXECS_NAOR_PINKAS);
	for(int i=0; i<NUM_EXECS_NAOR_PINKAS; i++)
		vKeys[i].Create(SHA1_BITS);
		 
	int sock_rcv;
	sha1_context sha1;
	CBitVector vRcv; 
	vRcv.AttachBuf(pBuf2);
		
	// buffer structure: [(s11, s12, s13, s14), ...., (s_np1, snp2, snp3, snp4)]_1, ...., [...]_and 
	int j, k, w, u;
	while( nProgress < m_nANDGates )
	{
		sock_rcv = min(nToRcvBytes, nWindowBytes);
		sock.Receive(pBuf2, sock_rcv);
		nToRcvBytes -= sock_rcv;
		 
		for(j=0, w=SHA1_BITS; j< OT_WINDOW_SIZE && nProgress < m_nANDGates; j++, nProgress++, w++)
		{ 	
			// prepare pad
			if(w == SHA1_BITS)
			{
				for(k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
				{
					sha1 = vKeySeeds[k];
					sha1_update(&sha1, (BYTE*) &nProgress, sizeof(nProgress));
					sha1_update(&sha1, (BYTE*) &k, sizeof(k));
					sha1_finish(&sha1, vKeys[k].GetArr());
				}
				w = 0;
			}

			// decrypt
			for(k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
			{
				u = U.Get2Bits(k);
				matrix[k].SetBit(nProgress, vKeys[k].GetBit(w)^vRcv.GetBit(j*4*NUM_EXECS_NAOR_PINKAS + k*4 + u));
			}
		}
	}
	
	vRcv.DetachBuf();
	delete [] pBuf2;

	return TRUE;
}

BOOL CParty::ThreadRunIKNPSndSecond(int nRcvID)
{
	// IKNP-second step: send (s0, s1, s2, s3)s
  //cout << " (" << nRcvID << ") starting IKNP second step" << endl;
	CSocket& sock = m_vSockets[nRcvID];
	CBitVector& input = m_vWWIn[nRcvID];
	CBitVector& U = m_vIKNPU[nRcvID];
	CBitMatrix& matrix = m_vIKNPMtx[nRcvID];
	int& nProgress = m_vIKNPProgress[nRcvID];

	CBitVector v;
	v.Create(OT_WINDOW_SIZE*4);
	int nOTBytes = (m_nANDGates*4 +7)/8;
	
	CBitVector qxors;
	qxors.Create(NUM_EXECS_NAOR_PINKAS);

	sha1_context sha;
	SHA_BUFFER sha_buf;
	int bit;
	int iu;
	
	int i=0;
	while(i<m_nANDGates)
	{
		int j=0;
		for( ; j<OT_WINDOW_SIZE && i<m_nANDGates; i++, j++)
		{
			while( i >= nProgress )
			{
				SleepMiliSec(100);
			}

			//ostringstream os;
			//os << " (" << nRcvID << ") IKNP: ot-input (" << i << ") = ";   
			for(int u=0; u<4; u++)
			{
				sha1_starts(&sha);
				iu = (i << 2) + u;
				sha1_update(&sha, (BYTE*) &iu, sizeof(iu));
	
				// gj xor s
				qxors.Reset();
				for(int k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
				{
					bit = matrix[k].GetBit(i);						// q
					bit ^= (U.Get2Bits(k) == u);	// s_j
					qxors.SetBit(k, bit);
				}
				sha1_update(&sha, qxors.GetArr(), NUM_EXECS_NAOR_PINKAS/8);
				sha1_finish(&sha, sha_buf);
		 
				v.SetBit(j*4+u, (sha_buf[0]&1) ^ input.GetBit(i*4+u));

				//os <<  (int) input.GetBit(i*4+u);
			}
			//os << endl;
			//cout << os.str() << flush;
		}
		
		sock.Send(v.GetArr(), (j*4+7)/8);
	}
	
	return TRUE;
}

 

BOOL CParty::ThreadRunIKNPRcvFirst(int nSndID)
{
	int nFieldSize = NumBytes(m_p);
	BYTE* pBuf = new BYTE[nFieldSize * NUM_EXECS_NAOR_PINKAS];

	//=================================================
	// N-P sender: send: C0 (=g^r), C1, C2, C3 
	ZZ* pC = new ZZ[4]; 
	
	// random k
	CBitVector rnd;
	rnd.Create(4*nFieldSize*8, m_aSeed, nSndID, m_vCounter[nSndID]);
	BYTE* pBufRnd = rnd.GetArr();

	// random C0 = g^r
	ZZ r, ztmp, ztmp2;
	ZZFromBytes(ztmp, pBufRnd, nFieldSize); 
	pBufRnd += nFieldSize;
	rem(r, ztmp, m_q);
	PowerMod(pC[0], m_g, r, m_p);
	
	// random C1
	ZZFromBytes(ztmp, pBufRnd, nFieldSize);
	pBufRnd += nFieldSize;
	rem(ztmp2, ztmp, m_p);
	SqrMod(pC[1], ztmp2, m_p);

	// random C2
	ZZFromBytes(ztmp, pBufRnd, nFieldSize);
	pBufRnd += nFieldSize;
	rem(ztmp2, ztmp, m_p);
	SqrMod(pC[2], ztmp2, m_p);

	// random C3
	ZZFromBytes(ztmp, pBufRnd, nFieldSize);
	pBufRnd += nFieldSize;
	rem(ztmp2, ztmp, m_p);
	SqrMod(pC[3], ztmp2, m_p);
	 
	int nBufSize = 4 * nFieldSize;	
	BYTE* pBufIdx = pBuf;
	for( int i=0; i<4; i++ )
	{
		BytesFromZZ(pBufIdx, pC[i], nFieldSize);
		pBufIdx += nFieldSize;
	}
	
	CSocket& sock = m_vSockets[nSndID];
	sock.Send(pBuf, nBufSize);
	
	//====================================================
	// compute C^R
	ZZ* pCr = new ZZ[4]; 
 	PowerMod(pCr[1], pC[1], r, m_p);
	PowerMod(pCr[2], pC[2], r, m_p);
	PowerMod(pCr[3], pC[3], r, m_p);
 
	//====================================================
	// N-P sender: receive pk0
	nBufSize = nFieldSize * NUM_EXECS_NAOR_PINKAS;
	sock.Receive(pBuf, nBufSize);

	ZZ* pPK0 = new ZZ[NUM_EXECS_NAOR_PINKAS];
	pBufIdx = pBuf;
	for(int i=0; i<NUM_EXECS_NAOR_PINKAS; i++)
	{
		ZZFromBytes(pPK0[i], pBufIdx, nFieldSize);
		pBufIdx += nFieldSize;
	}
	
	// NP sender: send Enc(M0), Enc(M1), Enc(M2), Enc(M3)
	delete [] pBuf;
	pBuf = new BYTE[NUM_EXECS_NAOR_PINKAS*4*OT_WINDOW_SIZE_BYTES];
	int nANDGatesBytes = (m_nANDGates+7)/8;
	 
	CBitVector& T = m_vIKNPT[nSndID];
	T.Create(NUM_EXECS_NAOR_PINKAS * m_nANDGates, m_aSeed, nSndID, m_vCounter[nSndID]);  
	
	CBitVector& input = m_vWWIn[nSndID];
	
	///========================================================
	/// Enc(M_u)s...
	// buffer structure: [(s11, s12, s13, s14), ...., (s_np1, snp2, snp3, snp4)]_1, ...., [...]_and 
	// Generate Key Seeds..
	vector< vector<sha1_context> > vKeySeedMtx;
	vKeySeedMtx.resize(NUM_EXECS_NAOR_PINKAS);
	for(int k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
	{
		vKeySeedMtx[k].resize(4); 
	}

	ZZ PK0r, PKr; //PK0r, pk^r;

	sha1_context* shap;
	for(int k=0; k<NUM_EXECS_NAOR_PINKAS; k++ )
	{
		for(int u=0; u<4; u++)
		{
			// pk^r
			if( u == 0 )
			{
				// pk0^r
				PowerMod(PK0r, pPK0[k], r, m_p);
 				BytesFromZZ(pBuf, PK0r, nFieldSize);
 
			}
			else
			{
				InvMod(ztmp, PK0r, m_p);
				MulMod(PKr, pCr[u], ztmp, m_p);
 				BytesFromZZ(pBuf, PKr, nFieldSize);
 			}

			shap = &vKeySeedMtx[k][u];
			sha1_starts(shap);
			sha1_update(shap, pBuf, nFieldSize);
		}
	}

	///========================================================
	/// Enc(M_u)s...
	// buffer structure: [(s11, s12, s13, s14), ...., (s_np1, snp2, snp3, snp4)]_1, ...., [...]_and 
	
#ifdef IMPL_PRIVIATE_CHANNEL
	{
		BYTE* key =  m_vPKeys[nSndID].GetArr();
		sha1_context sha2;
		int index = PKEY_SHA_INDEX;
		BYTE* pBufIdx = pBuf;
		for(int u=0; u<4; u++, pBufIdx += SHA1_BYTES)
		{ 
			sha2 = vKeySeedMtx[0][u];
			sha1_update(&sha2, (BYTE*) &index, sizeof(int));
			sha1_finish(&sha2, pBufIdx);
			for(int i=0; i<SHA1_BYTES; i++)
				pBufIdx[i] ^= key[i];
		}
		sock.Send(pBuf, SHA1_BYTES*4);
	}
#endif //IMPL_PRIVIATE_CHANNEL	

	vector<CBitMatrix> vKeyMtx(NUM_EXECS_NAOR_PINKAS);
	for(int k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
	{
		vKeyMtx[k].Create(4, SHA1_BITS);
	}

	CBitVector v;
	v.AttachBuf(pBuf);
	sha1_context sha1;
	int i=0;
	int j, k, u, w, t;
	int val;
	while( i < m_nANDGates )
	{
		for( j=0, w=SHA1_BITS; j<OT_WINDOW_SIZE && i < m_nANDGates; i++, j++, w++ )
		{
			// prepare pad
			if(w == SHA1_BITS)
			{
				for(k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
				{
					for(u=0; u<4; u++)
					{
						sha1 = vKeySeedMtx[k][u];
						sha1_update(&sha1, (BYTE*) &i, sizeof(i));
						sha1_update(&sha1, (BYTE*) &k, sizeof(k));
						sha1_finish(&sha1, vKeyMtx[k][u].GetArr());
					}
				}
				
				w = 0;
			}


			for(k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
			{
				t = T.GetBit(k*m_nANDGates+i);
				for(u=0; u<4; u++)
				{
					val = t;    
					val ^= (input.Get2Bits(i) == u);
					val ^= vKeyMtx[k][u].GetBit(w);
					v.SetBit( j*4*NUM_EXECS_NAOR_PINKAS + k*4 + u, val);
				}
			}
		}
		int nSize = (j*NUM_EXECS_NAOR_PINKAS*4 + 7)/8;
 		sock.Send( v.GetArr(), nSize );
		// receive done.
		m_vIKNPProgress[nSndID]++;
	}

	v.DetachBuf();
	delete [] pC;
	delete [] pCr;
	delete [] pPK0;
	delete [] pBuf; 

	return TRUE;
}

BOOL CParty::ThreadRunIKNPRcvSecond(int nSndID)
{
	
	// IKNP receiver: receive (s0, s1, s2, s3)s
	CBitVector& out = m_vWWOut[nSndID];
	CBitVector& input = m_vWWIn[nSndID];
	CBitVector& T = m_vIKNPT[nSndID];
	CSocket& sock = m_vSockets[nSndID];

	CBitVector v;
	v.Create(NUM_EXECS_NAOR_PINKAS*OT_WINDOW_SIZE*4);
		
	CBitVector tj;
	tj.Create(NUM_EXECS_NAOR_PINKAS);
	
	SHA_BUFFER sha_buf;
	sha1_context sha;

	int nWindowBytes = 4*OT_WINDOW_SIZE_BYTES;
	int nToRcvBytes = (4*m_nANDGates + 7)/8;
	 
	int sock_rcv;
	int i=0, j=0, iu=0, u=0; 
	
	// Wait until the first round recv is done
	// TODO: Aseem: May be this should go under while loop
	// this array is updated only when num and gates is > 0
	// Remember this may get stuck when 0 and gates
	/*while(!m_vIKNPProgress[nSndID])
	  SleepMiliSec(100);*/
	
	// buffer structure: [(s11, s12, s13, s14), ...., (s_np1, snp2, snp3, snp4)]_1, ...., [...]_and 
	while( i < m_nANDGates )
	{
	    /* Aseem : Not sure if this is right */
	while(!m_vIKNPProgress[nSndID])
		SleepMiliSec(100);

		sock_rcv = min(nToRcvBytes, nWindowBytes);
		sock.Receive(v.GetArr(), sock_rcv);
		
		nToRcvBytes -= sock_rcv;

		j=0;
		for(; j<OT_WINDOW_SIZE && i<m_nANDGates; i++, j++)
		{
			u = input.Get2Bits(i);
			iu = (i << 2) + u;
			sha1_starts(&sha);
			sha1_update(&sha, (BYTE*) &iu, sizeof(iu));
		 
			tj.Reset();
			for(int k=0; k<NUM_EXECS_NAOR_PINKAS; k++)
			{
				tj.SetBit(k, T.GetBit(k*m_nANDGates+i));
			}
			sha1_update(&sha, tj.GetArr(), NUM_EXECS_NAOR_PINKAS/8);
			sha1_finish(&sha, sha_buf);
		 	out.SetBit(i, (sha_buf[0]&1) ^ v.GetBit(j*4+u) );
			
			//cout << " (" << nSndID << ") IKNP: ot-output (" << i << "," << u <<") = " << (int) out.GetBit(i) << endl;
		 }
	}
	
	return TRUE;
}

//===========================================================================
// Multiplication Routines
BOOL CParty::ComputeMultiplications()
{	
	int nodeid, parentid;
	GATE *gate, *parent, *pleft, *pright;
	
	/*if(CConfig::GetInstance()->GetLoadOTNum() == 0) {
	    m_nMultIdx = 0;
	} else {
	    m_nMultIdx = CConfig::GetInstance()->GetLoadOTNum();
	    cout << "setting load ot num to " << m_nMultIdx  << endl;
	    }*/

	// OnGoing Que: inputs and determined values
	int nGateStart = m_pCircuit->GetGateStart();
	int nNumGates = m_pCircuit->GetNumGates(); 
	int nQueSize = max( nNumGates - m_nANDGates, m_nANDGates );  
	m_queOnGoings.Create(nQueSize);

	for(int i=2; i<nGateStart; i++ )
	{	
		m_queOnGoings.AddCurr(i);
	}

	char lval, rval;
	for(int i=nGateStart; i<nNumGates; i++ )
	{
	        gate = m_pGates + i;
		
		lval = m_pGates[gate->left].val;
		rval = m_pGates[gate->right].val;

		if( ( gate->type == G_XOR) &&
			IsValueAssigned(lval) && IsValueAssigned(rval) )
		{ 
			gate->val =  lval ^ rval; 
			m_queOnGoings.AddCurr(i);
		}
	}
	 
	//ostringstream os;
 	for(;;)
	{
		// Generate OnGoing queues
		/*
		os.str("");
		os << " queue: " << m_queOnGoings.CurrSize() << " nodes " << endl;
		cout << os.str() << flush;
		*/

	        while( m_queOnGoings.PopCurr(nodeid) )
		{ 
			gate = m_pGates + nodeid;  
			
			for(int j=0; j<gate->p_num; j++)
			{
				parentid = gate->p_ids[j];
				parent = m_pGates + parentid;
				pleft =  m_pGates + parent->left;
				pright = m_pGates + parent->right;
				
			
				if( parent->val == GATE_INITIALIZED &&
					IsValueAssigned(pleft->val) &&
					IsValueAssigned(pright->val) )
				{
						
					//cout <<"in the condition!" << endl;
					if( parent->type == G_XOR )
					{
						parent->val = pleft->val ^ pright->val;
						m_queOnGoings.AddCurr(parentid);
						//cout <<"xor gate: adding (" << parentid << ") to the curr que" << endl;
					}
					else
					{
						parent->val = GATE_ONGOING; 
						m_queOnGoings.AddAlt(parentid);
						//cout << "and gate: adding (" << parentid << ") to the alt queue" << endl;
						/*
						 cout << "parentid = " << parentid 
						<< ", type = "	<< (int) parent->type
						<< ", val = " << (int) parent->val
						<< ", leftid = " << parent->left
						<< ", leftval = " << (int) m_pGates[parent->left].val 
						<< ", rightid = " << parent->right
						<< ", rightval = " << (int) m_pGates[parent->right].val
						<< endl;
						*/
					}
				}
			}
		}

		
		
		
		m_queOnGoings.Alternate();
		if( m_queOnGoings.IsCurrEmpty() ) break;
	 
		/*
		os.str("");
		os << " queue: " << m_queOnGoings.CurrSize() << " nodes " << endl;
		cout << os.str() << flush;
		*/

		WakeupWorkerThreads(e_Mult);
		if( !WaitWorkerThreads() )
			return FALSE;

		nQueSize = m_queOnGoings.CurrSize();
		for(int i=0; i<nQueSize; i++)
		{
			m_queOnGoings.Peek(i, nodeid);
			gate = m_pGates + nodeid;
			gate->val = (m_nNumParties & 1) & m_pGates[gate->left].val & m_pGates[gate->right].val;
			for(int j = 0; j< m_nNumParties; j++ )
			{
				if( j != m_nPID ) gate->val ^= m_vGMWRnd[j].GetBit(m_nMultIdx + i); 
			}

			//cout << "gate (" << nodeid << "): feeding value " << (int) gate->val << endl;
		}
	
		m_nMultIdx += m_queOnGoings.CurrSize();

		//cout << "." << flush;
	}
	//cout << endl;

	return TRUE;
}


BOOL CParty::ThreadRunMultiplySender(int nRcvID)
{
	int nSize = m_queOnGoings.CurrSize();
	GATE* gate;
	int nodeid;

	CSocket& sock = m_vSockets[nRcvID];
	CBitVector vRcv, vSnd;
	vRcv.Create(nSize*2);
	vSnd.Create(nSize*4);

	int nRcvBytes = (nSize*2+7)/8;

	//ostringstream os;

	/*
	os << " (" << nRcvID << ") receiving  " << nRcvBytes << " bytes... " << endl;
	cout << os.str() << flush;
	*/
	sock.Receive(vRcv.GetArr(), nRcvBytes);
	

	BYTE a, b, c;
	BYTE _r;

	CBitVector& gmw = m_vGMWRnd[nRcvID];
	CBitVector& ww = m_vWWIn[nRcvID];

	int j=0, k4;
	for(int i=0,k=m_nMultIdx; i<nSize; i++, k++)
	{
		m_queOnGoings.Peek(i, nodeid);
		gate = m_pGates + nodeid;
		a = m_pGates[gate->left].val;
		b = m_pGates[gate->right].val;

		_r = vRcv.Get2Bits(i);
		c = gmw.GetBit(k);  
		
		k4 = k << 2; // *4 
		vSnd.SetBit(j++, c^(a&b)^(ww.GetBit(k4 + _r)) );
		vSnd.SetBit(j++, c^(a&(b^1))^(ww.GetBit(k4 + ((_r+3)&0x3))) );
		vSnd.SetBit(j++, c^((a^1)&b)^(ww.GetBit(k4 + ((_r+2)&0x3))) );
		vSnd.SetBit(j++, c^((a^1)&(b^1))^(ww.GetBit(k4 + ((_r+1)&0x3))) );
	
		/*
		os		<< " (" << nRcvID << ") " 
				<< "WW:ot-in(" << nodeid
				<< ") k=" << k
				<< " left=" << gate->left 
				<< " right=" << gate->right
				<< " abc =" << (int) a << (int) b << (int) c 
				<< " r=" << (int) _r 
				<< " "				
				<< (int) (c^(a&b)) << (int) (c^(a&(b^1)))
				<< (int) (c^((a^1)&b)) << (int) (c^((a^1)&(b^1)))
				<< " "
				<< (int) vSnd.GetBit(4*i)
				<< (int) vSnd.GetBit(4*i+1)
				<< (int) vSnd.GetBit(4*i+2)
				<< (int) vSnd.GetBit(4*i+3)
				<< endl;
		*/
	}

	int nSndBytes = (nSize*4+7)/8;

	//cout << os.str() << flush;

	/*
	os.str("");
	os << " (" << nRcvID << ") sending " << nSndBytes << " bytes" << endl;
	cout << os.str() << flush;
	*/

	sock.Send(vSnd.GetArr(), nSndBytes);

	return TRUE;
}

BOOL CParty::ThreadRunMultiplyReceiver(int nSndID)
{
	int nSize = m_queOnGoings.CurrSize();
	GATE* gate;
	int nodeid;

	CSocket& sock = m_vSockets[nSndID];
	CBitVector vSnd, vChoice, vRcv;  
	vSnd.Create(nSize*2);  
	vChoice.Create(nSize*2);
	vRcv.Create(nSize*4);

	int nSndBytes = (nSize*2+7)/8;
	CBitVector& gmw = m_vGMWRnd[nSndID];
	CBitVector& wwi = m_vWWIn[nSndID];
	CBitVector& wwo = m_vWWOut[nSndID];
	
	BYTE a, b;
	BYTE choice;
	for(int i=0, j=m_nMultIdx; i<nSize; i++, j++)
	{
		m_queOnGoings.Peek(i, nodeid);
		gate = m_pGates + nodeid;
		a = m_pGates[gate->left].val;
		b = m_pGates[gate->right].val;

		choice = ((a << 1) | (b & 1));
		vChoice.Set2Bits(i, choice);

		choice += wwi.Get2Bits(j);
		choice &= 0x3;
		vSnd.Set2Bits(i, choice);
	}

	//ostringstream os;
	
	/*
	os << " (" << nSndID << ") sending " << nSndBytes << " bytes" << endl;
	cout << os.str() << flush;
	*/

	sock.Send(vSnd.GetArr(), nSndBytes);

	int nRcvBytes = (nSize*4 - 1)/8 + 1;
	
	/*
	os.str("");
	os << " (" << nSndID << ") receiving " << nRcvBytes << " bytes" << endl;
	cout << os.str() << flush;
	*/
	sock.Receive(vRcv.GetArr(), nRcvBytes);
	
	BYTE sr;
	for(int i=0, j=m_nMultIdx; i<nSize; i++, j++)
	{
		m_queOnGoings.Peek(i, nodeid);
		gate = m_pGates + nodeid;
		a = m_pGates[gate->left].val;
		b = m_pGates[gate->right].val;

		choice = vChoice.Get2Bits(i);  
		sr = vRcv.GetBit(i*4 + choice);
		sr ^= wwo.GetBit(j);
		gmw.SetBit(j, sr); 

		/*
		os   << " (" << nSndID << ") "
			 <<"WW: ot out (" << nodeid
			 << ") j =" << j
			 << " left=" << gate->left 
			 << " right=" << gate->right
			 << " ab=" << (int) choice 
			 << " _r=" << (int) wwi.Get2Bits(i)
			 << " r=" << (int) vSnd.Get2Bits(i)
			 << " sr=" << (int) wwo.GetBit(i)
			 << " out= " << (int) sr << endl;
		*/
	}

	//cout << os.str() << flush;

	return TRUE;
}


//====================================================================
// Output Computing

BOOL CParty::ComputeOutput()
{
	/*
	for(int i=0; i<m_pCircuit->GetNumGates(); i++ )
	{
		GATE* gate = m_pCircuit->Gates() + i;
		cout << "gate(" << i <<"): type = " << (int) gate->type
			 << " leftval= " << ((gate->left >= 0)? m_pGates[gate->left].val : -1 )
			 << " rightval= " <<((gate->right >= 0)? m_pGates[gate->right].val: -1 )
			 << " val= " << (int) gate->val
			 << endl;
	}
	*/
	
	BOOL bSuccess;

	int o_start = m_pCircuit->GetOutputStart(m_nPID);
	int o_end = m_pCircuit->GetOutputEnd(m_nPID);
	int o_size = o_end - o_start + 1;
	
	if( o_start <= o_end )
	{	
		m_vOutputShares.resize(m_nNumParties);
		for(int i=0; i<m_nNumParties; i++ )
		{
			if( i != m_nPID )
				m_vOutputShares[i].Create(o_size);
		}
		
		m_vOutputs.resize(o_size);
		WakeupWorkerThreads(e_Output);
	}

	bSuccess = MainThreadRunSendOutputShare();
	
	if( o_start <= o_end )
	{
		WaitWorkerThreads();
	
		GATE* m_pGates = m_pCircuit->Gates();
	
		m_Outputs.Create(o_size);
		for(int i=0, idx=o_start; i<o_size; i++, idx++ )
		{
			m_Outputs.SetBit(i, m_pGates[idx].val);
			for(int j=0; j<m_nNumParties; j++ )
			{
				if( j == m_nPID ) continue;
				m_Outputs.XORBit(i, m_vOutputShares[j].GetBit(i));
			}
		}

		for(int i=0; i<o_size; i++)
		{
			m_vOutputs[i] = m_Outputs.GetBit(i);
		}
	}
    
	return bSuccess;
}


BOOL CParty::ThreadRunRecvOutputShare(int nSndID)
{
	CBitVector& v = m_vOutputShares[nSndID];
	int o_start = m_pCircuit->GetOutputStart(m_nPID);
	int o_end = m_pCircuit->GetOutputEnd(m_nPID);
	int o_size = o_end - o_start + 1;
	int nSizeBytes = (o_size+7)/8;
	
	/*
	cout << " ThreadRunRecvOutputShare(" << nSndID 
		 <<	"): receiving " << nSizeBytes << " bytes" << endl;
	*/
	m_vSockets[nSndID].Receive(v.GetArr(), nSizeBytes);

#ifdef IMPL_PRIVIATE_CHANNEL
	int pid = 0;
	int cnt = 0;
	CBitVector pad;
	pad.Create(o_size, m_vPKeys[nSndID].GetArr(), pid, cnt);
	for(int i=0, j=o_start; i<o_size; i++, j++ )
	{
		v.XORBit(i, pad.GetBit(i));
	}
#endif //IMPL_PRIVIATE_CHANNEL

	return TRUE;

}

BOOL CParty::MainThreadRunSendOutputShare()
{
	for(int Pi=0; Pi<m_nNumParties; Pi++)
	{
		if( Pi == m_nPID ) continue;
		
		int o_start = m_pCircuit->GetOutputStart(Pi);
		int o_end = m_pCircuit->GetOutputEnd(Pi);
		int o_size = o_end - o_start + 1;
		if( o_start > o_end ) continue;


		CBitVector v;
		int nSizeBytes = (o_size+7)/8;

#ifdef IMPL_PRIVIATE_CHANNEL
		int pid = 0;
		int cnt = 0;
		v.Create(o_size, m_vPKeys[Pi].GetArr(), pid, cnt);
		GATE* m_pGates = m_pCircuit->Gates();
		for(int i=0, j=o_start; i<o_size; i++, j++ )
		{
			v.XORBit(i, m_pGates[j].val );
		}
#else
		v.Create(o_size);
	 
		GATE* m_pGates = m_pCircuit->Gates();
		for(int i=0, j=o_start; i<o_size; i++, j++ )
		{
			v.SetBit(i, m_pGates[j].val );
		}
#endif //IMPL_PRIVIATE_CHANNEL
		m_vSockets[Pi].Send(v.GetArr(), nSizeBytes);
	}

	return TRUE;
}




//===========================================================================
// Thread Management
BOOL CParty::WakeupWorkerThreads(EJobType e)
{
    m_nWorkingThreads = m_nNumParties - 1;
    m_bWorkerThreadSuccess = TRUE;

    for( int i=0; i<m_nNumParties; i++ )
	{
	    if( i != m_nPID ) m_vThreads[i]->PutJob(e);
		
	}
    return TRUE;
}
 
BOOL CParty::WaitWorkerThreads()
{
	if( !m_nWorkingThreads ) return TRUE;

	for(;;)
	{
		m_lock.Lock();
		int n = m_nWorkingThreads;
		m_lock.Unlock();

		if(!n) return m_bWorkerThreadSuccess;
		m_evt.Wait();
	}

	return m_bWorkerThreadSuccess;
}

BOOL CParty::ThreadNotifyTaskDone(BOOL bSuccess)
{
	m_lock.Lock();
	int n = -- m_nWorkingThreads;
	if( !bSuccess ) m_bWorkerThreadSuccess = FALSE;
	m_lock.Unlock();

	if(!n) m_evt.Set();
	return TRUE;
}
 
void CParty::CWorkerThread::ThreadMain()
{
	CConfig* cf = CConfig::GetInstance();
	CParty* party = CParty::GetInstance();
	int nPID = party->m_nPID;
	int nNumParties = party->m_nNumParties;

	BOOL bSuccess;

	for(;;)
	{
		m_evt.Wait();
		 
		switch(m_eJob)
		{
		case e_Stop:
			return;
		case e_Connect:
			bSuccess = party->ThreadRunConnect(m_nPartnerID);
			break;
		case e_OTExt:
			if( AsOTSender(nPID, m_nPartnerID )) bSuccess = party->ThreadRunIKNPSndFirst(m_nPartnerID);
			else bSuccess = party->ThreadRunIKNPRcvFirst(m_nPartnerID);
			break;
		case e_ShareInput:
			bSuccess = party->ThreadRunRecvInputShare(m_nPartnerID);
			break;
		case e_Mult:
			if( AsOTSender(nPID, m_nPartnerID )) bSuccess = party->ThreadRunMultiplySender(m_nPartnerID);
			else bSuccess = party->ThreadRunMultiplyReceiver(m_nPartnerID);
			break;
		case e_Output:
			bSuccess = party->ThreadRunRecvOutputShare(m_nPartnerID);
			break;
		}
		party->ThreadNotifyTaskDone(bSuccess);
	}
}
	
void CParty::COTHelperThread::ThreadMain()
{
	CParty* party = CParty::GetInstance();
	//CConfig* cf = CConfig::GetInstance();
	int nPID = party->m_nPID;
	int nNumParties = party->m_nNumParties;

	if( AsOTSender(nPID,m_nPartnerID ))
		party->ThreadRunIKNPSndSecond(m_nPartnerID);
	else
		party->ThreadRunIKNPRcvSecond(m_nPartnerID);
}

