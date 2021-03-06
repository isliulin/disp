#include "wwtiny.h"

#include "dmpfmt.h"

using namespace std;

int print_MY_TYPE_SSEL2_Quotation(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SSEL2_Quotation   *p=(SSEL2_Quotation *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"createtime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->Time;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		case 4:	*plCurTime=(long)(p->PacketTimeStamp%MY_DATE_CEIL_LONG);break;
		default:*plCurTime=p->Time;break;
	}

	if(*plCurTime<lBgnTime) return 2;


	sprintf(outbuf,"%lld\t%d\t%lld\t%d\t%s\t%d\t%s\
\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		p->PacketTimeStamp,//< 数据包头时间YYYYMMDDHHMMSSMMM
		p->Time,	//< 数据生成时间, 最新订单时间（毫秒）;143025001 表示 14:30:25.001
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)p->Time),
		p->QuotationFlag,			//第一个续行
    		(int)yuan2percentFen(p->PreClosePrice),         ///< 昨收价,
    		(int)yuan2percentFen(p->OpenPrice),             ///< 开盘价,
    		(int)yuan2percentFen(p->HighPrice),             ///< 最高价,
    		(int)yuan2percentFen(p->LowPrice),              ///< 最低价,
    		(int)yuan2percentFen(p->LastPrice),             ///< 现价,
    		(int)yuan2percentFen(p->ClosePrice),            ///< 今收盘价,
		p->TradeStatus,    				///< 当前品种交易状态,
		p->SecurityPhaseTag,   				///< 当前品种状态
		(int)p->TotalNo,                		///< 成交笔数,
		(int)p->TotalVolume,           			///< 成交总量, 股票：股),权证：份),债券：手
    		(int)yuan2percentFen(p->TotalAmount),           ///< 成交总额, （元）
		(int)p->TotalBuyOrderVolume,    		///< 委托买入总量, 股票：股),权证：份),债券：手
    		(int)yuan2percentFen(p->WtAvgBuyPrice),         ///< 加权平均委买价, （元）
    		(int)yuan2percentFen(p->BondWtAvgBuyPrice),     ///< 债券加权平均委买价, （元）
		(int)p->TotalSellOrderVolume,		///< 委托卖出总量,
    		(int)yuan2percentFen(p->WtAvgSellPrice),        ///< 加权平均委卖价, （元）
    		(int)yuan2percentFen(p->BondWtAvgSellPrice),    ///< 债券加权平均委卖价,
    		(int)yuan2percentFen(p->IOPV),                  ///< ETF 净值估值,
		p->ETFBuyNo,				///< ETF 申购笔数,第二个续行
		(int)p->ETFBuyVolume, 			        ///< ETF 申购量,
    		(int)yuan2percentFen(p->ETFBuyAmount),          ///< ETF 申购额,
    		p->ETFSellNo,             			///< ETF 赎回笔数,
    		(int)p->ETFSellVolume,          		///< ETF 赎回量,
    		(int)yuan2percentFen(p->ETFSellAmount),         ///< ETF 赎回额,
    		(int)yuan2percentFen(p->YTM),                   ///< 债券到期收益率,
    		(int)p->TotalWarrantExecVol,    		///< 权证执行的总数量,
    		(int)yuan2percentFen(p->WarrantDownLimit),      ///< 权证跌停价格, （元）
    		(int)yuan2percentFen(p->WarrantUpLimit),        ///< 权证涨停价格, （元）
    		p->WithdrawBuyNo,          			///< 买入撤单笔数,
    		(int)p->WithdrawBuyVolume,      		///< 买入撤单量,
    		(int)yuan2percentFen(p->WithdrawBuyAmount),      ///< 买入撤单额,
    		p->WithdrawSellNo,			///< 卖出撤单笔数,
    		(int)p->WithdrawSellVolume,		///< 卖出撤单量,
    		(int)yuan2percentFen(p->WithdrawSellAmount),     ///< 卖出撤单额,
    		p->TotalBuyNo,             ///< 买入总笔数,
    		p->TotalSellNo,            ///< 卖出总笔数,
    		p->MaxBuyDuration,         ///< 买入成交最大等待时间,
    		p->MaxSellDuration,        ///< 卖出成交最大等待时间,
    		p->BuyOrderNo,             ///< 买方委托价位数,
    		p->SellOrderNo,            ///< 卖方委托价位数,	第三个续行
    		(int)p->SellLevelNo,   ///< 卖盘价位数量, 10档行情，不足时补空 第四个续行
    		(int)yuan2percentFen(p->SellPrice01),
    		(int)p->SellVolume01,
    		(int)p->TotalSellOrderNo01,
    		(int)yuan2percentFen(p->SellPrice02),
    		(int)p->SellVolume02,
    		(int)p->TotalSellOrderNo02,
    		(int)yuan2percentFen(p->SellPrice03),
    		(int)p->SellVolume03,
    		(int)p->TotalSellOrderNo03,
    		(int)yuan2percentFen(p->SellPrice04),
    		(int)p->SellVolume04,
    		(int)p->TotalSellOrderNo04,
    		(int)yuan2percentFen(p->SellPrice05),
    		(int)p->SellVolume05,
    		(int)p->TotalSellOrderNo05,	//第五个续行
    		(int)yuan2percentFen(p->SellPrice06),
    		(int)p->SellVolume06,
    		(int)p->TotalSellOrderNo06,
    		(int)yuan2percentFen(p->SellPrice07),
    		(int)p->SellVolume07,
    		(int)p->TotalSellOrderNo07,
    		(int)yuan2percentFen(p->SellPrice08),
    		(int)p->SellVolume08,
    		(int)p->TotalSellOrderNo08,
    		(int)yuan2percentFen(p->SellPrice09),
    		(int)p->SellVolume09,
    		(int)p->TotalSellOrderNo09,
    		(int)yuan2percentFen(p->SellPrice10),
    		(int)p->SellVolume10,
    		(int)p->TotalSellOrderNo10,	//第六个续行
    		(int)p->SellLevelQueueNo01,
    		(int)p->SellLevelQueue[0],
    		(int)p->BuyLevelNo,		//第七个续行
    		(int)yuan2percentFen(p->BuyPrice01),
    		(int)p->BuyVolume01,
    		(int)p->TotalBuyOrderNo01,
    		(int)yuan2percentFen(p->BuyPrice02),
    		(int)p->BuyVolume02,
    		(int)p->TotalBuyOrderNo02,
    		(int)yuan2percentFen(p->BuyPrice03),
    		(int)p->BuyVolume03,
    		(int)p->TotalBuyOrderNo03,
    		(int)yuan2percentFen(p->BuyPrice04),
    		(int)p->BuyVolume04,
    		(int)p->TotalBuyOrderNo04,
    		(int)yuan2percentFen(p->BuyPrice05),
    		(int)p->BuyVolume05,
    		(int)p->TotalBuyOrderNo05,	//第八个续行
    		(int)yuan2percentFen(p->BuyPrice06),
    		(int)p->BuyVolume06,
    		(int)p->TotalBuyOrderNo06,
    		(int)yuan2percentFen(p->BuyPrice07),
    		(int)p->BuyVolume07,
    		(int)p->TotalBuyOrderNo07,
    		(int)yuan2percentFen(p->BuyPrice08),
    		(int)p->BuyVolume08,
    		(int)p->TotalBuyOrderNo08,
    		(int)yuan2percentFen(p->BuyPrice09),
    		(int)p->BuyVolume09,
    		(int)p->TotalBuyOrderNo09,
    		(int)yuan2percentFen(p->BuyPrice10),
    		(int)p->BuyVolume10,
    		(int)p->TotalBuyOrderNo10,    		//第九个续行
    		(int)p->BuyLevelQueueNo01,
    		(int)p->BuyLevelQueue[0]
		);

	return 0;
}

int print_MY_TYPE_SSEL2_Transaction(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SSEL2_Transaction   *p=(SSEL2_Transaction *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"tradetime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->TradeTime;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		case 4:	*plCurTime=(long)(p->PacketTimeStamp%MY_DATE_CEIL_LONG); break;
		default:*plCurTime=p->TradeTime;break;
	}

	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%lld\t%d\t%s\t%d\
\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%c\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		p->PacketTimeStamp,//< 数据包头时间YYYYMMDDHHMMSSMMM
		p->TradeTime,	//< 成交时间(毫秒), 14302506 表示14:30:25.06
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)p->TradeTime),//第二行开始
		p->QuotationFlag,  			///< 1=上海南汇机房行情源；2=深圳福永机房行情源
		(int)p->RecID,                  	///< 业务索引, 从 1 开始，按 TradeChannel连续
		(int)p->TradeChannel,           	///< 成交通道,
		(int)yuan2percentFen(p->TradePrice),///< 成交价格,
		(int)p->TradeVolume,            	///< 成交数量, 股票：股,权证：份,债券：张
		(int)yuan2percentFen(p->TradeAmount),///< 成交金额,
		(int)p->BuyRecID,               	///< 买方订单号,
		(int)p->SellRecID,              	///< 卖方订单号,
		p->BuySellFlag            		///< 内外盘标志, B – 外盘,主动买,S – 内盘,主动卖,N – 未知
		);

	return 0;
}
int print_MY_TYPE_SSEL2_Auction(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SSEL2_Auction   *p=(SSEL2_Auction *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

//	TDF_MARKET_DATA m;

//	GTA2TDF_SSEL2_AM(p[0],m);

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"createtime",
			"stockcode",
			"difftime",
			"quotation_flag",
			"open_price",
			"auction_volume",
			"leave_volume",
			"side");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->Time;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		case 4:	*plCurTime=(long)(p->PacketTimeStamp%MY_DATE_CEIL_LONG);break;
		default:*plCurTime=p->Time;break;
	}

	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%lld\t%d\t%s\t%d\t%s\t%d\t%d\t%d\t%c\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		p->PacketTimeStamp,//< 数据包头时间YYYYMMDDHHMMSSMMM
		p->Time,	//< 数据生成时间, 143025001 表示 14:30:25.001
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)p->Time),
		p->QuotationFlag,
		(int)yuan2percentFen(p->OpenPrice),
		(int)p->AuctionVolume,
		(int)p->LeaveVolume,
		p->Side);

	return 0;
}
int print_MY_TYPE_SZSEL2_Quotation(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SZSEL2_Quotation   *p=(SZSEL2_Quotation *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

	*plCurTime=p->Time%MY_DATE_CEIL_LONG;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"createtime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->Time%MY_DATE_CEIL_LONG;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		default:*plCurTime=p->Time;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%ld\t%d\t%s\t%d\t%s\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		99999999999999999,//< 无数值
		(int)(p->Time%MY_DATE_CEIL_LONG),//< 数据生成时间YYYYMMDDHHMMSSMMM
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)(p->Time%MY_DATE_CEIL_LONG)),
		p->QuotationFlag,			//第一个续行
    		(int)yuan2percentFen(p->PreClosePrice),
    		(int)yuan2percentFen(p->OpenPrice),
    		(int)yuan2percentFen(p->LastPrice),
    		(int)yuan2percentFen(p->HighPrice),
    		(int)yuan2percentFen(p->LowPrice),
    		(int)yuan2percentFen(p->PriceUpLimit),
    		(int)yuan2percentFen(p->PriceDownLimit),
    		(int)yuan2percentFen(p->PriceUpdown1),
    		(int)yuan2percentFen(p->PriceUpdown2),
    		(int)p->TotalNo,
    		(int)yuan2percentFen(p->TotalVolume),
    		(int)yuan2percentFen(p->TotalAmount),
    		(int)yuan2percentFen(p->ClosePrice),
    		"0",//p->SecurityPhaseTag,
    		(int)yuan2percentFen(p->PERatio1),
    		(int)yuan2percentFen(p->NAV),
    		(int)yuan2percentFen(p->PERatio2),
    		(int)yuan2percentFen(p->IOPV),
    		(int)yuan2percentFen(p->PremiumRate),
    		(int)yuan2percentFen(p->TotalSellOrderVolume),
    		(int)yuan2percentFen(p->WtAvgSellPrice),//第二个续行
    		(int)p->SellLevelNo,   ///< 卖盘价位数量, 10档行情，不足时补空 第三个续行
    		(int)yuan2percentFen(p->SellPrice01),
    		(int)p->SellVolume01,
    		(int)p->TotalSellOrderNo01,
    		(int)yuan2percentFen(p->SellPrice02),
    		(int)p->SellVolume02,
    		(int)p->TotalSellOrderNo02,
    		(int)yuan2percentFen(p->SellPrice03),
    		(int)p->SellVolume03,
    		(int)p->TotalSellOrderNo03,
    		(int)yuan2percentFen(p->SellPrice04),
    		(int)p->SellVolume04,
    		(int)p->TotalSellOrderNo04,
    		(int)yuan2percentFen(p->SellPrice05),
    		(int)p->SellVolume05,
    		(int)p->TotalSellOrderNo05,	//第四个续行
    		(int)yuan2percentFen(p->SellPrice06),
    		(int)p->SellVolume06,
    		(int)p->TotalSellOrderNo06,
    		(int)yuan2percentFen(p->SellPrice07),
    		(int)p->SellVolume07,
    		(int)p->TotalSellOrderNo07,
    		(int)yuan2percentFen(p->SellPrice08),
    		(int)p->SellVolume08,
    		(int)p->TotalSellOrderNo08,
    		(int)yuan2percentFen(p->SellPrice09),
    		(int)p->SellVolume09,
    		(int)p->TotalSellOrderNo09,
    		(int)yuan2percentFen(p->SellPrice10),
    		(int)p->SellVolume10,
    		(int)p->TotalSellOrderNo10,	//第五个续行
    		(int)p->SellLevelQueueNo01,
    		(int)p->SellLevelQueue[0],
    		(int)yuan2percentFen(p->TotalBuyOrderVolume),
    		(int)yuan2percentFen(p->WtAvgBuyPrice),
    		(int)p->BuyLevelNo,		//第六个续行
    		(int)yuan2percentFen(p->BuyPrice01),
    		(int)p->BuyVolume01,
    		(int)p->TotalBuyOrderNo01,
    		(int)yuan2percentFen(p->BuyPrice02),
    		(int)p->BuyVolume02,
    		(int)p->TotalBuyOrderNo02,
    		(int)yuan2percentFen(p->BuyPrice03),
    		(int)p->BuyVolume03,
    		(int)p->TotalBuyOrderNo03,
    		(int)yuan2percentFen(p->BuyPrice04),
    		(int)p->BuyVolume04,
    		(int)p->TotalBuyOrderNo04,
    		(int)yuan2percentFen(p->BuyPrice05),
    		(int)p->BuyVolume05,
    		(int)p->TotalBuyOrderNo05,	//第七个续行
    		(int)yuan2percentFen(p->BuyPrice06),
    		(int)p->BuyVolume06,
    		(int)p->TotalBuyOrderNo06,
    		(int)yuan2percentFen(p->BuyPrice07),
    		(int)p->BuyVolume07,
    		(int)p->TotalBuyOrderNo07,
    		(int)yuan2percentFen(p->BuyPrice08),
    		(int)p->BuyVolume08,
    		(int)p->TotalBuyOrderNo08,
    		(int)yuan2percentFen(p->BuyPrice09),
    		(int)p->BuyVolume09,
    		(int)p->TotalBuyOrderNo09,
    		(int)yuan2percentFen(p->BuyPrice10),
    		(int)p->BuyVolume10,
    		(int)p->TotalBuyOrderNo10,    		//第八个续行
    		(int)p->BuyLevelQueueNo01,
    		(int)p->BuyLevelQueue[0],
    		(int)yuan2percentFen(p->WtAvgRate),
    		(int)yuan2percentFen(p->WtAvgRateUpdown),
    		(int)yuan2percentFen(p->PreWtAvgRate)
		);

	return 0;
}
int print_MY_TYPE_SZSEL2_Transaction(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SZSEL2_Transaction   *p=(SZSEL2_Transaction *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"tradetime",
			"stockcode",
			"difftime"
			);
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->TradeTime%MY_DATE_CEIL_LONG;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		default:*plCurTime=p->TradeTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%ld\t%d\t%s\t%d\
\t%s\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%c\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		99999999999999999,//< 数据包头时间YYYYMMDDHHMMSSMMM
		(int)(p->TradeTime%MY_DATE_CEIL_LONG),//成交时间YYYYMMDDHHMMSSMMM
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)(p->TradeTime%MY_DATE_CEIL_LONG)),//第二行开始
		p->QuotationFlag,  			///< 1=上海南汇机房行情源；2=深圳福永机房行情源
		(int)p->SetID,                  	///< 证券集代号
		(int)p->RecID,           		///< 消息记录号 从 1 开始计数，同一频道连续
		(int)p->BuyOrderID,                  	///< 买方委托索引
		(int)p->SellOrderID,           		///< 卖方委托索引
		p->SymbolSource,		///< 证券代码源
		(int)yuan2percentFen(p->TradePrice),///< 成交价格,
		(int)yuan2percentFen(p->TradeVolume),///< 成交金额,
		p->TradeType            		///< 成交类别：4=撤销，主动或自动撤单执行报告；F=成交，成交执行报告
		);

	return 0;
}

int print_MY_TYPE_SZSEL2_Order(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	SZSEL2_Order   *p=(SZSEL2_Order *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->Symbol)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\t%s\t%s\n",
			"picktime",
			"localtime",
			"packtime",
			"ordertime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->Time%MY_DATE_CEIL_LONG;break;
		case 2: *plCurTime=lTmpTime;break;
		case 3:	*plCurTime=p->LocalTimeStamp;break;
		default:*plCurTime=p->Time;break;
	}
	if(*plCurTime<lBgnTime) return 2;


	sprintf(outbuf,"%lld\t%d\t%ld\t%d\t%s\t%d\
\t%s\t%d\t%d\t%s\t%d\t%d\t%c\t%c\n",
		*(long long *)buf,//picktime
		p->LocalTimeStamp,//< 数据接收时间HHMMSSMMM
		99999999999999999,//< 数据包头时间YYYYMMDDHHMMSSMMM
		(int)(p->Time%MY_DATE_CEIL_LONG),//委托时间YYYYMMDDHHMMSSMMM
		p->Symbol,
		iDiffnTime((int)lTmpTime,(int)(p->Time%MY_DATE_CEIL_LONG)),//第二行开始
		p->QuotationFlag,  			///< 1=上海南汇机房行情源；2=深圳福永机房行情源
		(int)p->SetID,                  	///< 证券集代号
		(int)p->RecID,           		///< 消息记录号 从 1 开始计数，同一频道连续
		p->SymbolSource,		///< 证券代码源
		(int)yuan2percentFen(p->OrderPrice),///< 委托价格,4位小数
		(int)yuan2percentFen(p->OrderVolume),///< 委托数量,2位小数
		p->OrderCode,			///< 买卖方向：1=买 2=卖 G=借入 F=出借
		p->OrderType				///< 订单类别：1=市价 2=限价 U=本方最优
		);

	return 0;
}

int print_MY_TYPE_TDF_MKT(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	struct TDF_MARKET_DATA   *p=(struct TDF_MARKET_DATA *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->szCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"time",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->nTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=p->nTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;


	sprintf(outbuf,"%lld\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\
\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%d\t%d\n",
		*(long long *)buf,//picktime
		(int)p->nTime,	//委托时间YYYYMMDDHHMMSSMMM
		p->szCode,
		iDiffnTime((int)lTmpTime,(int)p->nTime),//第一行继续
		(int)p->nStatus,//状态
		(int)p->nPreClose,//前收盘
		(int)p->nOpen,	//开盘价
		(int)p->nHigh,	//最高价
		(int)p->nLow,	//最低价
		(int)p->nMatch,	//最新价	//第二行开始
		(int)p->nAskPrice[0],
		(int)p->nAskPrice[1],
		(int)p->nAskPrice[2],
		(int)p->nAskPrice[3],
		(int)p->nAskPrice[4],
		(int)p->nAskPrice[5],
		(int)p->nAskPrice[6],
		(int)p->nAskPrice[7],
		(int)p->nAskPrice[8],
		(int)p->nAskPrice[9],
		(int)p->nAskVol[0],
		(int)p->nAskVol[1],
		(int)p->nAskVol[2],
		(int)p->nAskVol[3],
		(int)p->nAskVol[4],
		(int)p->nAskVol[5],
		(int)p->nAskVol[6],
		(int)p->nAskVol[7],
		(int)p->nAskVol[8],
		(int)p->nAskVol[9],//第三行开始
		(int)p->nBidPrice[0],
		(int)p->nBidPrice[1],
		(int)p->nBidPrice[2],
		(int)p->nBidPrice[3],
		(int)p->nBidPrice[4],
		(int)p->nBidPrice[5],
		(int)p->nBidPrice[6],
		(int)p->nBidPrice[7],
		(int)p->nBidPrice[8],
		(int)p->nBidPrice[9],
		(int)p->nBidVol[0],
		(int)p->nBidVol[1],
		(int)p->nBidVol[2],
		(int)p->nBidVol[3],
		(int)p->nBidVol[4],
		(int)p->nBidVol[5],
		(int)p->nBidVol[6],
		(int)p->nBidVol[7],
		(int)p->nBidVol[8],
		(int)p->nBidVol[9],//第四行开始
		(int)p->nNumTrades,		//成交笔数
		(int)p->iVolume,		//成交总量
		(int)p->iTurnover,		//成交总金额
		(int)p->nTotalBidVol,		//委托买入总量
		(int)p->nTotalAskVol,		//委托卖出总量
		(int)p->nWeightedAvgBidPrice,	//加权平均委买价格
		(int)p->nWeightedAvgAskPrice,	//加权平均委卖价格
		(int)p->nIOPV,			//IOPV净值估值
		(int)p->nYieldToMaturity,	//到期收益率
		(int)p->nHighLimited,		//涨停价
		(int)p->nLowLimited,		//跌停价
		p->chPrefix,			//证券信息前缀
		(int)p->nSyl1,			//市盈率1
		(int)p->nSyl2,			//市盈率2
		(int)p->nSD2				//升跌2（对比上一笔）
		);

	return 0;
}

int print_MY_TYPE_TDF_TRA(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	struct TDF_TRANSACTION   *p=(TDF_TRANSACTION *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->szCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"tradetime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->nTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=p->nTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	if(p->chOrderKind==0) 	p->chOrderKind='0';
	else			p->chOrderKind='1';
	if(p->chFunctionCode==0) p->chFunctionCode='0';

	sprintf(outbuf,"%lld\t%d\t%s\t%d\
\t%d\t%d\t%d\t%d\t%c\t%c\t%c\t%d\t%d\n",
		*(long long *)buf,//picktime
		(int)p->nTime,//委托时间YYYYMMDDHHMMSSMMM
		p->szCode,
		iDiffnTime((int)lTmpTime,(int)p->nTime),//第二行开始
		(int)p->nIndex,            //成交编号
		(int)p->nPrice,            //成交价格
		(int)p->nVolume,        //成交数量
		(int)p->nTurnover,            //成交金额
		(char)p->nBSFlag,                    //买卖方向(买：'B', 卖：'S', 不明：' ')
		p->chOrderKind,                //成交类别
		p->chFunctionCode,             //成交代码
		(int)p->nAskOrder,                //叫卖方委托序号
		(int)p->nBidOrder	                //叫买方委托序号
		);

	return 0;
}

int print_MY_TYPE_TDF_ORD(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	struct TDF_ORDER   *p=(TDF_ORDER *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->szCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"ordertime",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->nTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=p->nTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	if(p->chOrderKind==0) p->chOrderKind='0';
	if(p->chFunctionCode==0) p->chFunctionCode='0';
	if(p->chStatus==0) 	p->chStatus='0';
	if(p->chFlag==0)	p->chFlag='0';

	sprintf(outbuf,"%lld\t%d\t%s\t%d\
\t%d\t%d\t%d\t%c\t%c\t%d\t%c\t%c\n",
		*(long long *)buf,//picktime
		(int)p->nTime,//委托时间YYYYMMDDHHMMSSMMM
		p->szCode,
		iDiffnTime((int)lTmpTime,(int)p->nTime),//第二行开始
		(int)p->nOrder,        	//委托号
		(int)p->nPrice,         	//委托价格
		(int)p->nVolume,	//委托数量
		p->chOrderKind,                //委托类别
		p->chFunctionCode,             //委托代码('B','S','C')
		(int)p->nBroker,	//经纪商编码(PT市场有效)
		p->chStatus,		//委托状态(PT市场有效)
		p->chFlag			//标志(PT市场有效)
		);

	return 0;
}

int print_MY_TYPE_TDF_QUE(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	struct TDF_ORDER_QUEUE   *p=(TDF_ORDER_QUEUE *)(buf+sizeof(long long));

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,p->szCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"time",
			"stockcode",
			"difftime");
	}

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);

	switch(iTimeFlag){
		case 1: *plCurTime=p->nTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=p->nTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%s\t%d\
\t%c\t%d\t%d\t%d\t%d\n",
		*(long long *)buf,//picktime
		(int)p->nTime,//委托时间YYYYMMDDHHMMSSMMM
		p->szCode,
		iDiffnTime((int)lTmpTime,(int)p->nTime),//第二行开始
		(char)p->nSide,		//买卖方向('B':Bid 'A':Ask)
		(int)p->nPrice,         	//委托价格
		(int)p->nOrders,	//订单数量
		(int)p->nABItems,	//明细个数
		(int)p->nABVolume[0]		//订单明细
		);

	return 0;
}

void MktData2TDF_MARKET_DATA(MktData &md, TDF_MARKET_DATA *po)
{
	bzero((void*)po,sizeof(TDF_MARKET_DATA));

	sprintf(po->szCode,"%06d",md.szcode());
	po->nTime=	md.ntime();
	po->nStatus=	md.nstatus();
	po->nPreClose=	md.npreclose();
	po->nOpen=	md.nopen();
	po->nHigh=	md.nhigh();
	po->nLow=	md.nlow();
	po->nMatch=	md.nmatch();

	for(int i=0;i<10;i++){
		po->nAskPrice[i]=	md.naskprice(i);
		po->nAskVol[i]=		md.naskvol(i);;
		po->nBidPrice[i]=	md.nbidprice(i);
		po->nBidVol[i]=		md.nbidvol(i);
	}

	po->nNumTrades=			md.nnumtrades();
	po->iVolume=                	md.ivolume();
	po->iTurnover=                  md.iturnover();
	po->nTotalBidVol=               md.ntotalbidvol();
	po->nTotalAskVol=               md.ntotalaskvol();
	po->nWeightedAvgBidPrice=       md.nweightedavgbidprice();
	po->nWeightedAvgAskPrice=       md.nweightedavgaskprice();
	po->nIOPV=                      md.niopv();
	po->nYieldToMaturity=           md.nyieldtomaturity();
	po->nHighLimited=               md.nhighlimited();
	po->nLowLimited=                md.nlowlimited();
	po->nSyl1=                      md.nsyl1();
	po->nSyl2=                      md.nsyl2();
	po->nSD2=                       md.nsd2();
}

int convert_MY_TYPE_TDF_MKT(char *pi,long lRecLen,char *po)
{
	string msg;

	MktData         md;

	msg.assign(pi, lRecLen);

	md.ParseFromString(msg);

	MktData2TDF_MARKET_DATA(md,(TDF_MARKET_DATA*)po);

	return 0;
}

void Transaction2TDF_TRANSACTION(Transaction &tr, TDF_TRANSACTION *po)
{
	bzero((void*)po,sizeof(TDF_TRANSACTION));

	sprintf(po->szCode,"%06d",tr.szcode());

	po->nTime=		tr.ntime();
	po->nIndex=		tr.nindex();
	po->nPrice=		tr.nprice();
	po->nVolume=		tr.nvolume();
	po->nTurnover=		tr.nturnover();
	po->nBSFlag=		tr.nbsflag();
	po->chOrderKind=	tr.chorderkind();
	po->chFunctionCode=	tr.chfunctioncode();
	po->nAskOrder=		tr.naskorder();
	po->nBidOrder=		tr.nbidorder();
}

int convert_MY_TYPE_TDF_TRA(char *pi,long lRecLen,char *po)
{
	string msg;

	Transaction         tr;

	msg.assign(pi, lRecLen);

	tr.ParseFromString(msg);

	Transaction2TDF_TRANSACTION(tr,(TDF_TRANSACTION*)po);

	return 0;
}
void Order2TDF_ORDER(Order &od, TDF_ORDER *po)
{

	bzero((void*)po,sizeof(TDF_ORDER));

	sprintf(po->szCode,"%06d",od.szcode());

	po->nTime=		od.ntime();

	po->nOrder=		od.norder();
	po->nPrice=		od.nprice();
	po->nVolume=		od.nvolume();


	po->chOrderKind=	od.chorderkind();
	po->chFunctionCode=	od.chfunctioncode();
	po->nBroker=		od.nbroker();
	po->chStatus=		od.chstatus();
	po->chFlag=		od.chflag();
}

int convert_MY_TYPE_TDF_ORD(char *pi,long lRecLen,char *po)
{
	string msg;

	Order         od;

	msg.assign(pi, lRecLen);

	od.ParseFromString(msg);

	Order2TDF_ORDER(od,(TDF_ORDER*)po);

	return 0;
}
void Order_queue2TDF_ORDER_QUEUE(Order_queue &oq, TDF_ORDER_QUEUE *po)
{
	bzero((void*)po,sizeof(TDF_ORDER_QUEUE));

	sprintf(po->szCode,"%06d",oq.szcode());

	po->nTime=		oq.ntime();
	po->nSide=		oq.nside()==1?'B':'A';
//	po->nSide=		(po->nSide==1?'B':'A');

	po->nPrice=		oq.nprice();
	po->nOrders=		oq.norders();
	po->nABItems=		oq.nabitems();

	for (int i = 0; i < po->nABItems; i++)
		po->nABVolume[i]=oq.nabvolume(i);
}
int convert_MY_TYPE_TDF_QUE(char *pi,long lRecLen,char *po)
{
	string msg;

	Order_queue         oq;

	msg.assign(pi, lRecLen);

	oq.ParseFromString(msg);

	Order_queue2TDF_ORDER_QUEUE(oq,(TDF_ORDER_QUEUE*)po);

	return 0;
}
int aiMyAmntLevel[10]={0,5,10,20,40,60,80,100,200,500};

int print_MY_TYPE_D31_STD(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	int l=0,i;
	char sCode[7];
	struct D31ItemStruct   *p=(D31ItemStruct *)(buf+sizeof(long long));

	sprintf(sCode,"%06d",p->nStockCode);

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,sCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"tradetime",
			"stockcode",
			"difftime");
	}

	char sTime[15];
	sFormatTime((time_t)p->nTradeTime,sTime);

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);
	long lTime=atoi(sTime+8)*1000;

	switch(iTimeFlag){
		case 1: *plCurTime=lTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=lTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	for(i=0;i<10;i++){
		l+=sprintf(outbuf+l,"%lld\t%d\t%s\t%d\
\t%s\t%4d\t%-10ld\t%-6d\t%-6d\t%-10ld\t%-6d\t%-6d\n",
			*(long long *)buf,//picktime
			(int)lTime,//委托时间YYYYMMDDHHMMSSMMM
			sCode,
			iDiffnTime((int)lTmpTime,(int)lTime),
			"z",aiMyAmntLevel[i],
			(long)(p->afZbBidAmount[i]*1000000),
			(int)(p->afZbBidVolume[i]*100),
			p->anZbBidOrderNum[i],
			(long)(p->afZbAskAmount[i]*1000000),
			(int)(p->afZbAskVolume[i]*100),
			p->anZbAskOrderNum[i]);

		l+=sprintf(outbuf+l,"%lld\t%d\t%s\t%d\
\t%s\t%4d\t%-10ld\t%-6d\t%-6d\t%-10ld\t%-6d\t%-6d\n",
			*(long long *)buf,//picktime
			(int)lTime,//委托时间YYYYMMDDHHMMSSMMM
			sCode,
			iDiffnTime((int)lTmpTime,(int)lTime),
			"w",aiMyAmntLevel[i],
			(long)(p->afZdBidAmount[i]*1000000),
			(int)(p->afZdBidVolume[i]*100),
			p->anZdBidOrderNum[i],
			(long)(p->afZdAskAmount[i]*1000000),
			(int)(p->afZdAskVolume[i]*100),
			p->anZdAskOrderNum[i]);

	}

	return 0;
}

int print_MY_TYPE_D31_EXT(char *buf,char sCodeList[],int iTimeFlag,
	long lBgnTime,long *plCurTime,char *outbuf)
{
	char sCode[7];
	struct D31ItemStruct   *p=(struct D31ItemStruct *)(buf+sizeof(long long));

	sprintf(sCode,"%06d",p->nStockCode);

	//如果代码列表非空且，本代码不在代码列表之内，则不打印
	if(sCodeList[0]&&
		strstr(sCodeList,sCode)==NULL) return 1;

	static int iFirstFlag=1;
	if(iFirstFlag){
		iFirstFlag=0;
		printf("%s\t%s\t%s\t%s\n",
			"picktime",
			"tradetime",
			"stockcode",
			"difftime");
	}

	char sTime[15];
	sFormatTime((time_t)p->nTradeTime,sTime);

	long lTmpTime=(long)((*(long long *)buf)%MY_DATE_CEIL_LONG);
	long lTime=atoi(sTime+8)*1000;

	switch(iTimeFlag){
		case 1: *plCurTime=lTime;break;
		case 2: *plCurTime=lTmpTime;break;
		default:*plCurTime=lTime;break;
	}
	if(*plCurTime<lBgnTime) return 2;

	sprintf(outbuf,"%lld\t%d\t%s\t%d\t\
%-10d\t%-10d\t%-12ld\t%-12ld\t%-10d\t%-10d\t%-12ld\t%-12ld\t\
%-10d\t%-10d\t%-10d\t%-10d\t%-12ld\t%-12ld\t%-12ld\t%-12ld\t%-12ld\t%-12ld\t%-12ld\t%-12ld\n",
		*(long long *)buf,//picktime		//1
		(int)lTime,//委托时间YYYYMMDDHHMMSSMMM	//2
		sCode,					//3
		iDiffnTime((int)lTmpTime,(int)lTime),	//4
		(int)(p->fTenBidVolume*100),		//5 十档买量（手）
		(int)(p->fTenAskVolume*100),		//6 十档卖量（手）
		(long)(p->fTenBidAmnt*1000000),		//7 十档买额（分）
		(long)(p->fTenAskAmnt*1000000),		//8 十档卖额（分）
		(int)(p->fTotalBidVolume*100),		//9 叫买总量（手）
		(int)(p->fTotalAskVolume*100),		//10叫卖总量（手）
		(long)(p->fTotalBidAmnt*1000000),	//11叫买总额（分）
		(long)(p->fTotalAskAmnt*1000000),	//12叫卖总额（分）
		(int)(p->fWtAvgBidPrice*10000),		//13加权平均叫买价（分）
		(int)(p->fWtAvgAskPrice*10000),		//14加权平均叫卖价（分）
		(int)(p->fLastClose*10000),		//15昨收盘价
		(int)(p->fCurPrice*10000),		//16最新价
		(long)(p->fAvgTotalBidAmnt*1000000),	//17平均叫买总额（当日平均，分）
		(long)(p->fAvgTotalAskAmnt*1000000),	//18平均叫卖总额（当日平均，分）
		(long)(p->fBidAmount20*1000000),	//19跳买额度20w，单位（分）
		(long)(p->fAskAmount20*1000000),	//20跳卖额度20w，单位（分）
		(long)(p->fBidAmount50*1000000),	//21跳买额度50w，单位（分）
		(long)(p->fAskAmount50*1000000),	//22跳卖额度50w，单位（分）
		(long)(p->fBidAmount100*1000000),	//23跳买额度100w，单位（分）
		(long)(p->fAskAmount100*1000000)	//24跳卖额度100w，单位（分）
	);

	return 0;
}

void D31Item2D31_ITEM(D31Item &di, struct D31ItemStruct *po)
{
	int i;

	bzero((void*)po,sizeof(struct D31ItemStruct));

	po->nStockCode=		di.nstockcode();
	po->nTradeTime=		di.ntradetime();

        for(i=0;i<10;i++) po->afZbBidAmount[i]=    di.afzbbidamount(i);
	for(i=0;i<10;i++) po->afZbBidVolume[i]=    di.afzbbidvolume(i);
	for(i=0;i<10;i++) po->anZbBidOrderNum[i]=  di.anzbbidordernum(i);
	for(i=0;i<10;i++) po->afZbAskAmount[i]=    di.afzbaskamount(i);
	for(i=0;i<10;i++) po->afZbAskVolume[i]=    di.afzbaskvolume(i);
	for(i=0;i<10;i++) po->anZbAskOrderNum[i]=  di.anzbaskordernum(i);
	for(i=0;i<10;i++) po->afZdBidAmount[i]=    di.afzdbidamount(i);
	for(i=0;i<10;i++) po->afZdBidVolume[i]=    di.afzdbidvolume(i);
	for(i=0;i<10;i++) po->anZdBidOrderNum[i]=  di.anzdbidordernum(i);
	for(i=0;i<10;i++) po->afZdAskAmount[i]=    di.afzdaskamount(i);
	for(i=0;i<10;i++) po->afZdAskVolume[i]=    di.afzdaskvolume(i);
	for(i=0;i<10;i++) po->anZdAskOrderNum[i]=  di.anzdaskordernum(i);

        po->fTenBidVolume=	di.ftenbidvolume();
	po->fTenAskVolume=       di.ftenaskvolume();
	po->fTenBidAmnt=         di.ftenbidamnt();
	po->fTenAskAmnt=         di.ftenaskamnt();
	po->fTotalBidVolume=     di.ftotalbidvolume();
	po->fTotalAskVolume=     di.ftotalaskvolume();
	po->fTotalBidAmnt=       di.ftotalbidamnt();
	po->fTotalAskAmnt=       di.ftotalaskamnt();
	po->fWtAvgBidPrice=      di.fwtavgbidprice();
	po->fWtAvgAskPrice=      di.fwtavgaskprice();
	po->fLastClose=          di.flastclose();
	po->fCurPrice=           di.fcurprice();
	po->fAvgTotalBidAmnt=    di.favgtotalbidamnt();
	po->fAvgTotalAskAmnt=    di.favgtotalaskamnt();
	po->fBidAmount20=        di.fbidamount20();
	po->fAskAmount20=        di.faskamount20();
	po->fBidAmount50=        di.fbidamount50();
	po->fAskAmount50=        di.faskamount50();
	po->fBidAmount100=       di.fbidamount100();
	po->fAskAmount100=       di.faskamount100();
}


int convert_MY_TYPE_D31_ITEM(char *pi,long lRecLen,char *po)
{
	string msg;

	D31Item         di;

	msg.assign(pi, lRecLen);

	di.ParseFromString(msg);

	D31Item2D31_ITEM(di,(struct D31ItemStruct*)po);

	return 0;
}
