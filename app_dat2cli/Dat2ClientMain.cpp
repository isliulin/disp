#include <string>
#include <iostream>
#include <time.h>
#include <errno.h>

#include "Dat2ClientMain.h"

using namespace std;

extern int errno;
static TcpSocket *g_pTcpSocket = NULL; //Tcp传输器

int iMyPid=0;

//将bizCode和pbmsg信息打包成为串 格式：2字节长度+1字节bizCode+pb->SerializeToString结果，长度为 1字节+序列化串
void addBizcode(string &output, const google::protobuf::Message &pbmsg, BizCode bizCode)
{
	string msg;

	pbmsg.SerializeToString(&msg);

	char strBizCode[2] = { 0 };

	strBizCode[0] = bizCode;
	output = string(strBizCode) + msg;

	uint16_t networkLenBody = uint16_t(htons(output.size()));

	char sLen[3]={0};

	sLen[0] = ((char*)&networkLenBody)[0];
	sLen[1] = ((char*)&networkLenBody)[1];
	output.insert(0,sLen,2);
}

//将msgInput的第一个字段取出，作为bizCode后面的串作为pbmsg
void getBizcode(BizCode &bizCode, string &pbmsg, const string &msgInput)
{
	bizCode = BizCode((unsigned char) (msgInput[0]));
	pbmsg = msgInput.substr(1, string::npos);
}

void startIosThread(boost::asio::io_service *pio)
{
	pio->run();
}
void disp_buf2(char *buf, int size)
{
	int i;

	printf("buf [%d] start:", size);
	for (i = 0; i < size; i++){
		if (i % 16 == 0) printf("\n");
		printf("%02X ", buf[i] & 0xff);
	}
	printf("\nbuf end\n\n");
}
void MyBin2HexStr(char *buf,int len,char sTemp[])
{
	int l=0;
	
	for(int i=0;i<len;i++){
		if(l==0)
			l+=sprintf(sTemp,"%02X",buf[i] & 0xff);
		else	l+=sprintf(sTemp+l," %02X",buf[i] & 0xff);
		if(l>=253) break;
	}
}

char sLogTime[15],sMSec[4];

int GetHostTimeX(char sHostTime[15],char sMiniSec[4])
{
	struct tm *tm;
	struct timeval t;
	time_t tHostTime;

	strcpy(sMiniSec,"");

	if(gettimeofday(&t,NULL)==-1) return -1;

	tHostTime=(time_t)(t.tv_sec);
	if((tm=(struct tm*)localtime(&tHostTime))==NULL) return -1;

	if(strftime(sHostTime,15,"%Y%m%d%H%M%S",tm)==(size_t)0)	return -1;

	sprintf(sMiniSec,"%03ld",t.tv_usec/1000);

	return 0;
}

bool Dat2Client::m_bReadSysMq = true;
bool Dat2Client::m_bExitFlag = false;

void signalProcess(int isignal)
{
	if (isignal == SIGUSR1){
		Dat2Client::m_bReadSysMq = true;
		return;
	}

	if (isignal == SIGINT||isignal==SIGTERM){
		Dat2Client::m_bExitFlag = true;
		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s pid=%d(%d) CATCH SIG=%d.\n",
			sLogTime,sMSec,getpid(),iMyPid,isignal);
		
	//只有是子进程的情况下才退出，如果是首要进程，则不直接下去exit
		if(getpid()!=iMyPid) return;
	}
	exit(0);
}

Dat2Client::Dat2Client()
{
	m_iSemLockKey = 0;

	memset(m_sUserPrivJsonPath, 0, sizeof(m_sUserPrivJsonPath));
	memset(m_sOutDispJsonPath, 0, sizeof(m_sOutDispJsonPath));
	memset(m_sCfgJsonPath, 0, sizeof(m_sCfgJsonPath));
	
	memset(m_sWorkRootPath, 0, sizeof(m_sWorkRootPath));
	memset(m_sWriteUser, 0, sizeof(m_sWriteUser));


	strcpy(m_sWorkRootPath,"/stock/work");

	m_iPort = 0; 		//侦听端口
	m_iSocketSendLen = 8096;
	m_iSocketRecvLen = 8096;
	m_iSysMqKey = 0;
	m_iSysMqMaxLen = 0;
	m_iSysMqMaxNum = 0;

	m_iDebugFlag = 0;
	m_iCheckIp = 0;
	m_iSetSocketFlag = 0;
	m_iPid = 0;

	m_poSysMQ = NULL;
	m_poDataMQ = NULL;

	m_pTimerController = NULL;
	m_pThread = NULL;

	m_mapPrivl.clear();
	m_secHeartbeat = 0;
	m_sUserName = "";
	m_allCodes.clear();
	m_Subscribed.clear();
	m_iMqID = 0;
	m_subSomeCodes = false;
	m_loginOK = false;
	m_codes.clear();

#ifdef DEBUG_ONE
	m_lRecvNum = 0;
	m_lSendNum = 0;
	m_mapBizStat.clear();
#endif
}

Dat2Client::~Dat2Client()
{
	m_io.stop();

	m_mapPrivl.clear();
	if (m_poSysMQ != NULL){
		delete m_poSysMQ;
		m_poSysMQ = NULL;
	}
	if (m_poDataLock != NULL){
		delete m_poDataLock;
		m_poDataLock = NULL;
	}
	if (m_pTimerController != NULL){
		delete m_pTimerController;
		m_pTimerController = NULL;
	}
	if (m_pThread != NULL){
		delete m_pThread;
		m_pThread = NULL;
	}
	if (m_iPid != 0)
		kill(m_iPid, SIGUSR1);
}

bool Dat2Client::isValidHost(const char *sIp)
{
	//sIp验证
	for (auto it = m_mapPrivl.begin(); it != m_mapPrivl.end(); ++it){
		if (!strcmp(it->second.m_sIp.c_str(), sIp))
			return true;
	}
	return false;
}

int Dat2Client::Init()
{
	ptree tree;

	//读取系统配置信息
	read_json(m_sCfgJsonPath, tree);
	m_iSemLockKey = tree.get<int>("SemLockKey");

	m_iPort = 		tree.get<int>("Port");
	m_iSocketSendLen = 	tree.get<int>("SocketSendLen");
	m_iSocketRecvLen = 	tree.get<int>("SocketRecvLen");
	m_iSysMqKey = 		tree.get<int>("SysMqID");
	m_iSysMqMaxLen = 	tree.get<int>("SysMqMaxLen");
	m_iSysMqMaxNum = 	tree.get<int>("SysMqMaxNum");
	m_secHeartbeat = 	tree.get<int>("heartbeat");

	m_poSysMQ = new MessageQueue(m_iSysMqKey);
	if (!m_poSysMQ)
		throw "error";
	m_poSysMQ->open(false, true, m_iSysMqMaxLen, m_iSysMqMaxNum);

	char sSemName[32];

	sprintf(sSemName, "%d", m_iSemLockKey);
	m_poDataLock = new CSemaphore();
	m_poDataLock->getSem(sSemName, 1, 1);

	read_json(m_sUserPrivJsonPath, tree);

	Privilege prl;
	ptree ptreePrivl;

	for (auto it = tree.begin(); it != tree.end(); ++it){

		auto each = it->second;

		prl.m_sUser = each.get < string > ("user");
		prl.m_sIp = each.get < string > ("ip");
		prl.password = each.get < string > ("pswd");
		prl.m_iMqID = each.get<int>("mqid");
		ptreePrivl = each.get_child("privl");

		for (auto it2 = ptreePrivl.begin(); it2 != ptreePrivl.end(); ++it2)
			prl.privl.insert(it2->second.get_value<int>());

		m_mapPrivl.insert( { prl.m_sUser, prl });
		prl.privl.clear();
	}

	return 0;
}

int Dat2Client::diffTimeStr(char *curr_time_str, const char *last_time_str)
{
	int diff_time,curr_fraction, last_fraction;
	struct tm curr_tm, last_tm;
	time_t last_time, curr_time;

	sscanf(curr_time_str, "%d-%d-%d %d:%d:%d.%d", &curr_tm.tm_year, &curr_tm.tm_mon, &curr_tm.tm_mday,
		&curr_tm.tm_hour, &curr_tm.tm_min, &curr_tm.tm_sec, &curr_fraction);
	curr_tm.tm_year -= 1900;
	curr_time = mktime(&curr_tm);

	sscanf(last_time_str, "%d-%d-%d %d:%d:%d.%d", &last_tm.tm_year, &last_tm.tm_mon, &last_tm.tm_mday,
		&last_tm.tm_hour, &last_tm.tm_min, &last_tm.tm_sec, &last_fraction);
	last_tm.tm_year -= 1900;
	last_time = mktime(&last_tm);

	diff_time = (curr_time - last_time) * 1000 + (curr_fraction - last_fraction) / 1000;

	return diff_time;
}

int Dat2Client::run(int argc, char *argv[])
{
	SetProcessSignal();
	for (int c; (c = getopt(argc, argv, "c:p:d:k:r:u:o:w:?:")) != EOF;){
		switch (c)
		{
		case 'c':
			m_iCheckIp = atoi(optarg);
			break;
		case 'd':
			m_iDebugFlag = atoi(optarg);
			break;
		case 'k':
			m_iSetSocketFlag = atoi(optarg);
			break;
		case 'p':
			strcpy(m_sCfgJsonPath, optarg);
			break;
		case 'r':
			strcpy(m_sOutDispJsonPath, optarg);
			sprintf(m_sOutDispLogPath,"%s.log",m_sOutDispJsonPath);
			break;
		case 'u':
			strcpy(m_sUserPrivJsonPath, optarg);
			break;
		case 'o':
			strcpy(m_sWorkRootPath, optarg);
			break;
		case 'w':
			strcpy(m_sWriteUser, optarg);
			break;
		case '?':
		default:
			printf("Usage: %s \n", argv[0]);
			printf("   [-p cfg-path ]\n");
			printf("   [-r disp-path ]\n");
			printf("   [-u user-privlegepath ]\n");
			printf("   [-o work-root-path ]\n");
			printf("   [-w write-user ]\n");
			printf("   [-d DebugFlag ]\n");
			printf("   [-c 1-Check IP;0-UnCheck IP ]\n");
			printf("   [-k 1:set socket buffer len 0: do not set socket buffer len]\n");
			exit(1);
			break;
		}
	}

	Init();

	int ret,iPort;
	long iDat2CliPid, iCliReqProcPid;
	char sDestIp[31] = { 0 };


	if (signal(SIGCHLD, SIG_DFL) != SIG_ERR)
		signal(SIGCHLD, SIG_IGN);

	g_pTcpSocket = new TcpSocket;
	struct sockaddr_in remoteaddr;

	while (true){

		ret = g_pTcpSocket->listen(m_iPort, remoteaddr);
		if (ret < 0 && ret != -2) return -1;

		sleep(1);
		g_pTcpSocket->CloseListenSocket();

		//校验接入IP是否合法
		iPort=ntohs(remoteaddr.sin_port);
		strcpy(sDestIp, inet_ntoa(remoteaddr.sin_addr));

		GetHostTimeX(sLogTime,sMSec);

		if (m_iCheckIp)	{
			if (!isValidHost(sDestIp)){

				printf("%s.%s 未知IP:%s;拒绝接入.\n",sLogTime,sMSec,sDestIp);

				g_pTcpSocket->Close();
				continue;
			}
		}
		printf("%s.%s IP:%s;接入成功.\n",sLogTime,sMSec,sDestIp);

		if (m_iSetSocketFlag){
			if (m_iSocketSendLen > 0)
				g_pTcpSocket->setSocketBufferLen(true, m_iSocketSendLen);

			if (m_iSocketRecvLen > 0)
				g_pTcpSocket->setSocketBufferLen(false, m_iSocketRecvLen);
		}

		//int m_iPid; //收发进程记录与它配对应的PID 主进程记录为0
		switch (fork()){
		case -1://fork失败情况
			exit(1);
			break;
		case 0:
			iDat2CliPid = getpid();
//			cout << "["<<sSysDate<<"]:"<<"连接成功%s:"<<inet_ntoa(remoteaddr.sin_addr)<<":"<<ntohs(remoteaddr.sin_port)<<"connected"<<endl;

			GetHostTimeX(sLogTime,sMSec);
			printf("%s.%s IP:连接成功%s:%dconnected.\n",sLogTime,sMSec,sDestIp,iPort);

			iCliReqProcPid = fork();
			switch (iCliReqProcPid)
			{
			case -1:
				exit(1);
				break;
			case 0:
				//处理客户端发送的订阅请求进程
				m_iPid = iDat2CliPid;
				ret=RecvCliSockRequestAndProcess(sDestIp,iPort);
				g_pTcpSocket->Close();
				logout();
				
				kill(iDat2CliPid, SIGINT);
				GetHostTimeX(sLogTime,sMSec);
				printf("%s.%s CLICMD EXIT user=%s(%s:%d-p-%d) code=%d.\n",
					sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid(),ret);
					
				break;
			default://转发MQ数据到客户端SOCK连接进程

				m_iPid = iCliReqProcPid;

				ret=RecvMqAndDisp2Cli(sDestIp,iPort);
				g_pTcpSocket->Close();
				kill(iCliReqProcPid, SIGINT);

				GetHostTimeX(sLogTime,sMSec);
				printf("%s.%s RECVMQ EXIT user=%s(%s:%d-p-%d) code=%d.\n",
					sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid(),ret);
				break;
			}

			if (m_iDebugFlag)
				printf("exit child process [%d]\n", getpid());
			return 0;
			break;
		default:
			//startMonitorMq();
			g_pTcpSocket->Close();
			break;
		}
	}

	g_pTcpSocket->CloseListenSocket();
	g_pTcpSocket->Close();

	delete g_pTcpSocket;

	return 0;
}

int Dat2Client::RecvCliSockRequestAndProcess(char sDestIp[],int iPort)
{
	string msg;
	int recv_len, msg_len, ret;
	unsigned char *data = (unsigned char*)m_sRecvBuffer;


	GetHostTimeX(sLogTime,sMSec);
	printf("%s.%s CLICMD process start user=%s(%s:%d-p-%d).\n",
		sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

	memset(m_sRecvBuffer, 0, sizeof(m_sRecvBuffer));

	while(!Dat2Client::m_bExitFlag){

		msg_len = 2;
		recv_len = errno = 0;

		//做一个循环，从客户端socket中读取消息长度data中
		while(recv_len < msg_len){
			if ((ret = g_pTcpSocket->read((data + recv_len), msg_len - recv_len)) <= 0){
				GetHostTimeX(sLogTime,sMSec);
				printf("%s.%s user=%s(%s:%d-p-%d) READ ERROR code=%d.\n",
					sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid(),ret);

				return ERROR_TRANSPORT;
			}
			recv_len += ret;
		}

		msg_len = data[0] * 256 + data[1];

		//校验消息长度的合法性，如果非法则断开
		if (msg_len <= 0||msg_len > (SOCKET_DATALEN - 2)){ //消息长度有误
			printf("%s.%s 消息长度有误，msg_len：%d!\n",sLogTime,sMSec,msg_len);
			return ERROR_TRANSPORT;
		}


#ifdef DEBUG_ONE
		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s user=%s(%s:%d-p-%d) Recv BizCode[%d]Msg_Len[%d].\n",
			sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid(),(int)data[3],msg_len);
#endif

		//得到有效的消息长度后，则读取整个消息
		while(recv_len - 2 < msg_len){
			if ((ret = g_pTcpSocket->read((unsigned char *) (data + recv_len), msg_len)) <= 0){
				
				GetHostTimeX(sLogTime,sMSec);
				
				printf("%s.%s g_pTcpSocket读取失败.\n",sLogTime,sMSec);
				
				return ERROR_TRANSPORT;
			}
			recv_len += ret;
		}

		msg.assign((char*)data+2, msg_len);

		if(dealCommand(msg,sDestIp,iPort)==false){
			printf("%s.%s user=%s(%s:%d-p-%d) PROCESS CMD ERROR.\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

			return ERROR_TRANSPORT;
		}
	}
	return SUCC;
}
int UserInWriteUser(char sUserName[],char sWriteUser[])
{
	char sTmpUser[MAX_PATH];
	char *pBgn,*pCur;
	
	//如果没有输出列表，则不输出
	if(sWriteUser[0]==0) return 0;

	strcpy(sTmpUser,sWriteUser);
	
	pBgn=sTmpUser;
	
	pCur=strchr(pBgn,':');
	
	while(1){
		if(pCur==NULL) break;

		*pCur=0;
		if(strcmp(sUserName,pBgn)==0) return true;
		
		//跳过':'号
		pBgn=pCur+1;
		pCur=strchr(pBgn,':');
	}

	//严格完全匹配呀
	if(strcmp(sUserName,pBgn)==0) return true;
	return false;
}
FILE *fpOut;
char sOutFileName[MAX_PATH];

int MyWrite2CliFile(char sWorkRoot[],char sUserName[],string &str)
{
	//只考虑linux了，long long 太难看了
	long lCurTime;
	static int iFirstFlag=true;
	
	char sHostTime[15],sMiniSec[4];
	
	GetHostTimeX(sHostTime,sMiniSec);

	if(iFirstFlag){
		iFirstFlag=false;
		char sTmpDate[9];
		strncpy(sTmpDate,sHostTime,8);sTmpDate[8]=0;
		
		sprintf(sOutFileName,"%s/%s_%s.dat",sWorkRoot,sUserName,sTmpDate);
		
		fpOut=	fopen(sOutFileName,"ab+");
		
		if(fpOut==NULL){
			printf("open file %s for write error.\n",sOutFileName);
			return -1;
		}
	}
	lCurTime=atol(sHostTime)*1000+atol(sMiniSec);
	
	if(fwrite((const void*)&lCurTime,sizeof(lCurTime),1,fpOut)!=1){
		printf("write file %s error pos1.\n",sOutFileName);
		return -1;
	}
	if(fwrite((const void*)str.c_str(),str.size(),1,fpOut)!=1){
		printf("write file %s error pos2.\n",sOutFileName);
		return -1;
	}

	fflush(fpOut);

	return 0;
}
int (*Write2CliFile)(char sWorkRoot[],char sUserName[],string &str);

int Dat2Client::RecvMqAndDisp2Cli(char sDestIp[],int iPort)
{
	int iRet = 0,iDataMqID = 0,iCount=0;
	string strRecv;

	GetHostTimeX(sLogTime,sMSec);
	printf("%s.%s RECVMQ process user=%s(%s:%d-p-%d).\n",
		sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

	while (!Dat2Client::m_bExitFlag){

		if (m_bReadSysMq){ //总控队列用于接收MQID，通过信号触发

			//接收登录的MQ信号，得到MQID
			if((iRet = m_poSysMQ->receive(strRecv, m_iPid))!=0){
				m_bReadSysMq = false;
				iDataMqID = atoi(strRecv.c_str());
				m_poDataMQ = new MessageQueue(iDataMqID);

				if (!m_poDataMQ)
					throw "error";

				m_poDataMQ->open(false, true, m_iSysMqMaxLen, m_iSysMqMaxNum);

				//从系统消息队列中拿到用户名
				char *pUserName=strchr((char*)strRecv.c_str(),',');
				if(pUserName==NULL) return ERROR_TRANSPORT;

				m_sUserName=string(pUserName+1);

				if(UserInWriteUser((char*)m_sUserName.c_str(),m_sWriteUser))
					Write2CliFile=MyWrite2CliFile;
				else	Write2CliFile=NULL;
				continue;
			}

			//如果没收到队列的MQ信号，就休眠一下等待
			usleep(10000);
			continue;
		}

		//这里poDataMQ已经准备好了
		//如果接收到消息为空，没有消息，则休眠0.1ms继续接收
		if((iRet = m_poDataMQ->receive(strRecv, 0))<=0){
			usleep(100);
			continue;
		}

		//做一个统计信息输出到终端
		m_lRecvNum++;
		BizCode bizCode = BizCode((unsigned char) (strRecv[2]));
		m_mapBizStat[bizCode]++;

		if(m_lRecvNum%3000==0){

			int l=0;
			char sTmp[256];

			map<BizCode,long>::iterator iter;//定义一个迭代指针iter
				
			strcpy(sTmp,"");

			GetHostTimeX(sLogTime,sMSec);

			for(iter=m_mapBizStat.begin(); iter!=m_mapBizStat.end(); iter++){
				if(l==0)
					l+=sprintf(sTmp,"[%d:%d]",(int)iter->first,(int)iter->second);
				else	l+=sprintf(sTmp+l,"\t[%d:%d]",(int)iter->first,(int)iter->second);
			}

			printf("%s.%s pid=%d user=%s(%s:%d-p-%d)sendcnt=%ld{%s}\n",
				sLogTime,sMSec,getpid(),m_sUserName.c_str(),sDestIp,iPort,getpid(),m_lRecvNum,sTmp);
		}

		unsigned char *data = (unsigned char*)m_sSendBuffer;
		int msg_len = strRecv.size(),num = 0;

		memcpy(m_sSendBuffer, strRecv.c_str(), strRecv.size());

		while(num < msg_len){
			if((iRet = g_pTcpSocket->write((data + num), msg_len - num))<0){
				printf("write socket error count=%d.\n",iCount);
				return ERROR_TRANSPORT;
			}
			num += iRet;
		}

		if(Write2CliFile!=NULL){
			if(Write2CliFile(m_sWorkRootPath,(char*)m_sUserName.c_str(),strRecv)<0)
				return ERROR_TRANSPORT;
		}
		if(((++iCount)%30000)==0)
			printf("pid=%d,the mqid=%d\tprocessed count=%d.\n",getpid(),m_poDataMQ->m_oriKey,iCount);
	}
	return SUCC;
}

void Dat2Client::SetProcessSignal()
{
	// 设置信号处理
	signal(SIGQUIT, signalProcess);
	signal(SIGTERM, signalProcess);
	signal(SIGINT, signalProcess);
	signal(SIGSEGV, signalProcess);
	signal(SIGILL, signalProcess);
	signal(SIGABRT, signalProcess);
	signal(SIGFPE, signalProcess);
	signal(SIGPIPE, signalProcess);

	signal(SIGUSR1, signalProcess);
}

bool Dat2Client::dealCommand(string &msg,char sDestIp[],int iPort)
{
	bool bret = true;
	BizCode iBizCode;
	string msgProtobuf;

	Reply rep;
	string msgRep,msgBody;

	getBizcode(iBizCode, msgProtobuf, msg);

	switch (iBizCode){

	case LOGIN_REQ:
	{
		LoginRequest req;
		req.ParseFromString(msgProtobuf);

		ErrCode errcode = ErrCode::OTHER;

		const string pswdInput = req.password();
		const auto itPrivl = m_mapPrivl.find(req.name());

		m_sUserName = req.name();

		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s Recv LOGIN_REQ user=%s(%s:%d-p-%d) passwd=%s.\n",
			sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid(),pswdInput.c_str());
		disp_buf2((char*)(msg.c_str()),msg.size());

		m_iMqID = itPrivl->second.m_iMqID;

		rep.set_err(errcode);
		//先查询用户名是否有效,无效则返回客户端
		if (itPrivl == m_mapPrivl.cend()){
			rep.set_desc("user_invalid");
			addBizcode(msgBody, rep, BizCode::LOGIN_REP);
			g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());
			bret = false;
			printf("%s.%s LOGIN_REP invalid user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

			break;
		}
		//如果已经登录则，回复已经登录信息
		if(isLogined(m_sUserName)){
			rep.set_desc("user_have_logined");
			addBizcode(msgBody, rep, BizCode::LOGIN_REP);
			g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());
			bret = false;
			printf("%s.%s LOGIN_REP user logined user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());
			break;
		}
		//密码错误，则将密码错误信息返回客户端
		if(itPrivl->second.password != req.password()){
			rep.set_desc("pswd_error");
			addBizcode(msgBody, rep, BizCode::LOGIN_REP);
			g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());
			bret = false;

			printf("%s.%s LOGIN_REP passwd error user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());
			break;
		}

		//用户，密码验证通过了再发送mqid给接收进程
		char sMsg[128];
		sprintf(sMsg, "%d,%s", m_iMqID,m_sUserName.c_str());
		string tmpStr=sMsg;
		m_poSysMQ->send(tmpStr, getpid());

		//如果密码验证也通过，则将心跳代码加入
		if(!m_pTimerController){
			//启动定时器
			m_pTimerController  = new TimerController(m_io,boost::bind(&Dat2Client::handleTimeOut,this),m_secHeartbeat*2);
			//std::thread([this]{ m_io.run(); });
			//std::thread trd(startIosThread,&m_io);
			m_pThread = new std::thread(startIosThread,&m_io);
			m_pThread->detach();
		}
		else{
			m_pTimerController->resetTimer(m_secHeartbeat*2);
		}

		//如果密码验证也通过，并在返回成功后，再返回客户端m_allCodes信息【pb格式】
		rep.set_err(ErrCode::SUCCESS);
		char tmp[6];
		sprintf(tmp, "%u", m_secHeartbeat);
		msgRep = tmp;

		rep.set_desc(msgRep);
		addBizcode(msgBody, rep, BizCode::LOGIN_REP);

		g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());

		PbCodesBroadcast pbCodesBroadcast;
		for (const auto code : m_allCodes){
			pbCodesBroadcast.add_codes(code);
		}

		string pbMsg2;

		addBizcode(pbMsg2, pbCodesBroadcast, BizCode::CODES_BROADCAST);
		g_pTcpSocket->send((unsigned char *) pbMsg2.data(), pbMsg2.size());
		
		printf("%s.%s LOGIN_REP login SUCCESS user=%s(%s:%d-p-%d).\n",
			sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

		m_loginOK=true;
	}
	break;
	case SUBSCRIBLE:
	{
		char sInfo[256],sMsg[256],sErrMsg[256];
	
		SubscribeRequest req;
		
		//如果没有过登录这个环节，必须提示并拒绝登录
		if(m_loginOK!=true){
			rep.set_desc("must_logined");
			addBizcode(msgBody, rep, BizCode::LOGIN_REP);
			g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());
			bret = false;

			printf("%s.%s SUBSCRIBLE_REP must logined first user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());
			break;
		}
		
		req.ParseFromString(msgProtobuf);
		bret = setSubscrible(req);

		GetHostTimeX(sLogTime,sMSec);

		sprintf(sInfo,"Recv SUBSCRIBLE user=%s(%s:%d-p-%d) Msg:Sub_Size=%d",
			m_sUserName.c_str(),sDestIp,iPort,getpid(),req.sub_size());

		printf("%s.%s %s.\n",sLogTime,sMSec,sInfo);

		disp_buf2((char*)(msg.c_str()),msg.size());
		
		//这里将改变之后的文件，全量写入到 disp.json.log文件中
		MyBin2HexStr((char*)(msg.c_str()),msg.size(),sMsg);
		if(LogDispJson(sInfo,sMsg,sErrMsg)==false){
			GetHostTimeX(sLogTime,sMSec);
			printf("%s.%s SUBSCRIBLE %s user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,sErrMsg,m_sUserName.c_str(),sDestIp,iPort,getpid());
			bret = false;
		}
	}
		break;
	case HEART_BEAT:
	{

		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s Recv HEART_BEAT MSG user=%s(%s:%d-p-%d).\n",
			sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());

		disp_buf2((char*)(msg.c_str()),msg.size());

		m_pTimerController->resetTimer(m_secHeartbeat*2);
	}
		break;
	case CODES_SUB:
	{
		
		char sInfo[256],sMsg[256],sErrMsg[256];

		PbCodesSub req;

		//如果没有过登录这个环节，必须提示并拒绝登录
		if(m_loginOK!=true){
			rep.set_desc("must_logined");
			addBizcode(msgBody, rep, BizCode::LOGIN_REP);
			g_pTcpSocket->send((unsigned char *) msgBody.data(), msgBody.size());
			bret = false;

			printf("%s.%s CODES_SUB_REP must logined first user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());
			break;
		}

		req.ParseFromString(msgProtobuf);
		const int sz = req.codes_size();

		vector<uint32_t> codes(sz);
		const auto reqCodes = req.codes();
		for (int i = 0; i < sz; ++i)
		{
			codes[i] = reqCodes.operator[](i);
		}
		bret = addReduceCodes(codes, req.add_red());

		GetHostTimeX(sLogTime,sMSec);
		sprintf(sInfo,"Recv CODES_SUB user=%s(%s:%d-p-%d) Msg:codes_size=%d",
			m_sUserName.c_str(),sDestIp,iPort,getpid(),sz);

		printf("%s.%s %s.\n",sLogTime,sMSec,sInfo);

		disp_buf2((char*)(msg.c_str()),msg.size());
		
		//这里将改变之后的文件，全量写入到 disp.json.log文件中
		MyBin2HexStr((char*)(msg.c_str()),msg.size(),sMsg);
		if(LogDispJson(sInfo,sMsg,sErrMsg)==false){
			GetHostTimeX(sLogTime,sMSec);
			printf("%s.%s CODES_SUB %s user=%s(%s:%d-p-%d).\n",
				sLogTime,sMSec,sErrMsg,m_sUserName.c_str(),sDestIp,iPort,getpid());
			bret = false;
		}
	}
		break;
	default:
	{
		bret = false;
		
		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s Unknow Msg user=%s(%s:%d-p-%d).\n",
			sLogTime,sMSec,m_sUserName.c_str(),sDestIp,iPort,getpid());
		disp_buf2((char*)(msg.c_str()),msg.size());

		break;
	}
	}
	return bret;
}
#define MY_MAX_JSON_LEN	16*1024
long lFileSize(char sFileName[])//获取文件名为filename的文件大小。
{
	struct stat statbuf;
	int ret;
	ret = stat(sFileName,&statbuf);//调用stat函数
	if(ret != 0) return -1;//获取失败。
	return statbuf.st_size;//返回文件大小。
}
bool Dat2Client::LogDispJson(char sInfo[],char sMsg[],char sErrInfo[])
{
	FILE *fpOut,*fpIn;
	long lSize;
	char sBuffer[MY_MAX_JSON_LEN];

	strcpy(sErrInfo,"");

	lSize=lFileSize(m_sOutDispJsonPath);
	if(lSize>=MY_MAX_JSON_LEN){
		sprintf(sErrInfo,"FILE %s LEN GT %d",m_sOutDispJsonPath,MY_MAX_JSON_LEN);
		return false;
	}
	if((fpIn= fopen(m_sOutDispJsonPath,"r"))==NULL){
		sprintf(sErrInfo,"OPEN %s FILE FOR READ ERROR",m_sOutDispJsonPath);
		return false;
	}

	if(fread((void*)sBuffer,lSize,1,fpIn)!=1){
		fclose(fpIn);
		sprintf(sErrInfo,"READ %s FILE ERROR",m_sOutDispJsonPath);
		return false;
	}
	fclose(fpIn);
	
	//将文件末尾加入结束符，避免将冗余内容写入日志文件
	sBuffer[lSize]=0;

	if((fpOut= fopen(m_sOutDispLogPath,"ab+"))==NULL){
		sprintf(sErrInfo,"OPEN %s FILE FOR WRITE ERROR",m_sOutDispLogPath);
		return false;
	}

	//拿到系统时间，并将信息写到DISPLOG文件中
	GetHostTimeX(sLogTime,sMSec);
	fprintf(fpOut,"%s.%s INFO:%s MSG:%s\nDISPJSON=%s ::------------END-----------\n",
		sLogTime,sMSec,sInfo,sMsg,sBuffer);
	
	fclose(fpOut);
	
	return true;
}
bool Dat2Client::setSubscrible(const SubscribeRequest &req)
{
	if(m_sUserName.size() == 0)
		return false;

	if (!req.replay()){

		const auto pri = m_mapPrivl.find(m_sUserName);
		const int numSub = req.sub_size();
		BizCode bizCodeTmp;

		m_Subscribed.clear();

		for (int i = 0; i < numSub; ++i){

			auto t=pri->second.privl;

			bizCodeTmp = BizCode(req.sub(i));
			
			//如果权限找到,对于D31的订购，分别找各个子项目
			if(bizCodeTmp==D31_ITEM){
				if(t.find(180)!=t.cend())	m_Subscribed.insert(180);
				if(t.find(183)!=t.cend())	m_Subscribed.insert(183);
				if(t.find(185)!=t.cend())	m_Subscribed.insert(185);
				if(t.find(18)!=t.cend())	m_Subscribed.insert(18);
			}
			else{
				if(t.find(bizCodeTmp)!=t.cend())m_Subscribed.insert((int)bizCodeTmp);
			}
//			if (pri->second.privl.find(bizCodeTmp) != pri->second.privl.cend()){
//				m_Subscribed.insert(bizCodeTmp); //note去掉了4个subcode的insert
//			}
		}

//		m_Subscribed.insert(FOR_TEST);
		writeDispJson();
	}
	return true;
}

bool Dat2Client::addReduceCodes(const vector<uint32_t> &codes, const bool addFlag)
{
	if(m_sUserName.size() == 0)
		return false;

	m_subSomeCodes = true;

	for (const auto code : codes){
		if(addFlag)
			m_codes.insert(code);
		else	m_codes.erase(code);
	}

	writeDispJson();
	m_poDataLock->V();
	return true;
}

bool Dat2Client::writeDispJson()
{
	string str = "";
	char sTmp[56] = { 0 };
	bool bfound = false;

	m_poDataLock->P();

	ptree root, readPt, items, userparam, itemsub, itemsubcodes, root_1;
	read_json(m_sOutDispJsonPath, readPt);

	if (readPt.count("users"))
	{
		ptree ptChildRead = readPt.get_child("users");
		for (auto pos : ptChildRead) //遍历数组
		{
			ptree p = pos.second;
			string user = p.get < string > ("user");
			if (user != m_sUserName)
			{
				root_1.push_back(pos);
			}
			else
			{
				userparam.put("user", m_sUserName);
				userparam.put<int>("mqid", m_iMqID);
				userparam.put<int>("pid", getpid());

				for (set<int>::iterator iter = m_Subscribed.begin(); iter != m_Subscribed.end(); ++iter)
				{
					sprintf(sTmp, "%u", *iter);
					str = sTmp;
					itemsub.push_back(std::make_pair("", ptree(str)));
				}
				userparam.put_child("subscribed", itemsub);

				for (set<uint32_t>::iterator iter = m_codes.begin(); iter != m_codes.end(); ++iter)
				{
					sprintf(sTmp, "%u", *iter);
					str = sTmp;
					itemsubcodes.push_back(std::make_pair("", ptree(str)));
				}
				userparam.put_child("subcodes", itemsubcodes);
				root_1.push_back(std::make_pair("", userparam));
				bfound = true;
			}
		}
		if (!bfound)
		{
			userparam.put("user", m_sUserName);
			userparam.put<int>("mqid", m_iMqID);
			userparam.put<int>("pid", getpid());

			for (set<int>::iterator iter = m_Subscribed.begin(); iter != m_Subscribed.end(); ++iter)
			{
				sprintf(sTmp, "%u", *iter);
				str = sTmp;
				itemsub.push_back(std::make_pair("", ptree(str)));
			}
			userparam.put_child("subscribed", itemsub);

			for (set<uint32_t>::iterator iter = m_codes.begin(); iter != m_codes.end(); ++iter)
			{
				sprintf(sTmp, "%u", *iter);
				str = sTmp;
				itemsubcodes.push_back(std::make_pair("", ptree(str)));
			}
			userparam.put_child("subcodes", itemsubcodes);
			root_1.push_back(std::make_pair("", userparam));

		}
		root.put_child("users", root_1);
		write_json(m_sOutDispJsonPath, root);
	}
	else
	{
		userparam.put("user", m_sUserName);
		userparam.put<int>("mqid", m_iMqID);
		userparam.put<int>("pid", getpid());

		for (set<int>::iterator iter = m_Subscribed.begin(); iter != m_Subscribed.end(); ++iter)
		{
			sprintf(sTmp, "%u", *iter);
			str = sTmp;
			itemsub.push_back(std::make_pair("", ptree(str)));
		}
		userparam.put_child("subscribed", itemsub);

		for (set<uint32_t>::iterator iter = m_codes.begin(); iter != m_codes.end(); ++iter)
		{
			sprintf(sTmp, "%u", *iter);
			str = sTmp;
			itemsubcodes.push_back(std::make_pair("", ptree(str)));
		}
		userparam.put_child("subcodes", itemsubcodes);
		root_1.push_back(std::make_pair("", userparam));
		root.put_child("users", root_1);
		write_json(m_sOutDispJsonPath, root);
	}

	m_poDataLock->V();
	return true;

}
bool Dat2Client::isLogined(string sUserName)
{
	string str = "";
	bool bfound = false;

	m_poDataLock->P();

	ptree root, readPt, items, userparam, itemsub, itemsubcodes, root_1;
	read_json(m_sOutDispJsonPath, readPt);
	if (readPt.count("users")){
		ptree ptChildRead = readPt.get_child("users");
		for (auto pos : ptChildRead){ //遍历数组

			ptree p = pos.second;
			string user = p.get < string > ("user");
			if (user != sUserName){
				root_1.push_back(pos);
			}
			else{
				int iPid = p.get<int>("pid",0);
				if (iPid == 0 || 0 != kill(iPid, 0)){ //不存在

					userparam.put("user", m_sUserName);
					userparam.put("pid", getpid());
					root_1.push_back(std::make_pair("", userparam));
					bfound = true;
				}
				else{
					m_poDataLock->V();
					return true;
				}
			}
		}
		if (!bfound){
			userparam.put("user", m_sUserName);
			userparam.put<int>("pid", getpid());
			root_1.push_back(std::make_pair("", userparam));

		}
		root.put_child("users", root_1);
//		这里不能写文件，避免catch异常出错如下:
//		watch exception pid=126635
//		write_json(m_sOutDispJsonPath, root);
	}
	else{
		userparam.put("user", m_sUserName);
		userparam.put<int>("pid", getpid());

		root_1.push_back(std::make_pair("", userparam));
		root.put_child("users", root_1);
		write_json(m_sOutDispJsonPath, root);
	}

	m_poDataLock->V();
	return false;

}
void Dat2Client::handleTimeOut()
{
	GetHostTimeX(sLogTime,sMSec);
	
	printf("%s.%s Client TimeOut! Pid[%d] will exit!",sLogTime,sMSec,getpid());

	kill(m_iPid, SIGINT);
	logout();
	exit(0);
}

void Dat2Client::logout()
{	
	string str = "";

	ptree root, readPt, items, userparam, itemsub, itemsubcodes, root_1;
	
	m_poDataLock->P();

	read_json(m_sOutDispJsonPath, readPt);
	
	if (readPt.count("users") && m_sUserName.size()>0 )
	{
		ptree ptChildRead = readPt.get_child("users");
		for (auto pos : ptChildRead) //遍历数组
		{
			ptree p = pos.second;
			string user = p.get < string > ("user");
			if (user != m_sUserName)
			{
				root_1.push_back(pos);
			}
			else
			{
				int iPid = p.get<int>("pid",0);
				if (iPid != 0 && iPid!=getpid() && 0 == kill(iPid, 0)) //不存在
				{
					m_poDataLock->V();
					return ;
				}
			}
		}
		root.put_child("users", root_1);
		write_json(m_sOutDispJsonPath, root);
	}


	//LOGOUT时，将文件备份一下
	char sInfo[256],sErrMsg[256];

	GetHostTimeX(sLogTime,sMSec);

	sprintf(sInfo,"Recv LOGOUT user=%s(p-%d)",m_sUserName.c_str(),getpid());

	printf("%s.%s %s.\n",sLogTime,sMSec,sInfo);

	if(LogDispJson(sInfo,(char*)"NULL",sErrMsg)==false){
		GetHostTimeX(sLogTime,sMSec);
		printf("%s.%s LOGOUT %s user=%s(p-%d).\n",
		sLogTime,sMSec,sErrMsg,m_sUserName.c_str(),getpid());
		exit(1);
	}

	m_poDataLock->V();
}

void Dat2Client::GetSysDate(char sSysDate[])
{
	struct tm *tim;
	time_t ltim;
	time(&ltim);
	tim = localtime(&ltim);
	sprintf(sSysDate,"%04d%02d%02d%02d%02d%02d",tim->tm_year+1900,tim->tm_mon+1,tim->tm_mday,
	        tim->tm_hour,tim->tm_min,tim->tm_sec);
}

void Dat2Client::runMonitorMqThread(const int iMqid)
{
#ifdef DEBUG_ONE
	cout<<"start monitor mq ID["<<iMqid<<"]"<<endl;
#endif
	MqMonitorMgr oMonitorMgr(iMqid);
	oMonitorMgr.startMqmonitor();
}

void Dat2Client::startMonitorMq()
{
	static bool bInit = false;
	if (bInit)
		return;
	bInit = true;
	thread_group group;
	for (auto it = m_mapPrivl.begin(); it != m_mapPrivl.end(); ++it)
	{
		group.create_thread(bind(&Dat2Client::runMonitorMqThread, it->second.m_iMqID));
	}
	//group.join_all();
}

int main(int argc, char *argv[])
{
	iMyPid=getpid();

	Dat2Client oTransMgr;
	oTransMgr.run(argc, argv);
}
