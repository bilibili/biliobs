#include "BiliDanmakuHime.hpp"
#include "BiliGlobalDatas.hpp"
#include <WinSock2.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <sstream>

#include <assert.h>

#include "third_party/w32-pthreads/pthread.h"

//#define DANMKAUSERVERADDR "livecmt-1.bilibili.com"
#define DANMKAUSERVERADDR gBili_danmakuServer.c_str()
//#define DANMKAUSERVERADDR "172.16.0.238"
#define DANMAKUSERVERPORT 788

#define BEFORE_RECONNECT_WAITTIME 3000

#define SOCKETTIMEOUT_THRESHOLD 10

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#else
#define closesocket close
#endif


#pragma region(方便使用的socket函数)

static bool GetHostAddress(SOCKADDR_IN* addr, const char* host)
{
	hostent* hst = gethostbyname(host);
	if (hst == 0)
		return false;

	if (hst->h_length > 0)
	{
		memcpy(&addr->sin_addr, hst->h_addr, sizeof(addr->sin_addr));
		return true;
	}
	else
		return false;
}

class SocketIOError{};

static int sendSocketData(socket_t s, const char* p, int l)
{
	int pos = 0;
	while (pos < l)
	{
		int r = send(s, &p[pos], l - pos, 0);
		if (r < 0)
			throw SocketIOError();
		else
			pos += r;
	}
	return pos;
}

enum { WAITTYPE_READ, WAITTYPE_WRITE };

template<int type>
static int WaitSocket(socket_t s, double timeout = SOCKETTIMEOUT_THRESHOLD)
{
	struct fd_set fs;
	FD_ZERO(&fs);
	FD_SET(s, &fs);

	struct fd_set fse = fs;

	struct timeval tv;
	tv.tv_sec = static_cast<int>(timeout);
	tv.tv_usec = static_cast<int>((timeout - tv.tv_sec) * 1000000);

	int r;
	if (type == WAITTYPE_READ)
		r = select(2, &fs, 0, &fse, &tv);
	else if (type == WAITTYPE_WRITE)
		r = select(2, 0, &fs, &fse, &tv);

	if (r == 1 && type == WAITTYPE_READ)
	{
		u_long dataSize;
		ioctlsocket(s, FIONREAD, &dataSize);
		return dataSize;
	}
	else if (r == 1 && type == WAITTYPE_WRITE)
		return 1;
	else if (r == 0)
		return 0;
	else if (r < 0)
		throw SocketIOError();

	return -1; //不应该会执行到这一行代码
}

static int ReadSocketFixedByte(socket_t s, void* pbuf, int len, double timeout = SOCKETTIMEOUT_THRESHOLD)
{
	int p = 0;
	char* buf = static_cast<char*>(pbuf);
	while (p < len)
	{
		int r = WaitSocket<WAITTYPE_READ>(s, timeout);
		if (r > 0)
		{
			int readbyte = recv(s, &buf[p], len - p, 0);
			if (readbyte > 0)
				p += readbyte;
			else if (readbyte == 0) //服务器关闭了连接
				throw SocketIOError();
			else if (readbyte < 0) //坏了
				throw SocketIOError();
		}
		else
			throw SocketIOError();
	}
	return p;
}

#pragma endregion


#pragma region(GOIM的数据包操作类)

namespace {
	class GoimPacket
	{
	public:
		enum { REQ_HEARTBEAT = 2, RESP_HEARTBEAT = 3, RESP_MESSAGE = 5, REQ_AUTH = 7, RESP_AUTH = 8 };

		class PacketHeader
		{
		private:
			std::uint32_t packet_length;
			std::uint16_t header_length;
			std::uint16_t protocol_version;
			std::uint32_t command;
			std::uint32_t seq;

		public:
			PacketHeader()
			{
				std::fill_n((char*)this, sizeof(*this), 0);
			}

			std::uint32_t get_packet_length() const { return ntohl(packet_length); }
			void set_packet_length(uint32_t val) { packet_length = htonl(val); }

			std::uint16_t get_header_length() const { return ntohs(header_length); }
			void set_header_length(std::uint16_t val) { header_length = htons(val); }

			std::uint16_t get_protocol_version() const { return ntohs(header_length); }
			void set_protocol_version(std::uint16_t val) { protocol_version = htons(val); }

			std::uint32_t get_command() const { return ntohl(command); }
			void set_command(std::uint32_t val) { command = htonl(val); }

			std::uint32_t get_seq() const { return ntohl(seq); }
			void set_seq(std::uint32_t val) { seq = htonl(val); }

			std::uint32_t get_payload_length() const 
			{
				return get_packet_length() - get_header_length();
			}
		};

		GoimPacket(int seq, int command)
		{
			InitHeader();

			header().set_seq(seq);
			header().set_command(command);
		}

		GoimPacket(const GoimPacket& rhs)
		{
			*this = rhs;
		}

		GoimPacket(GoimPacket&& rhs)
		{
			*this = std::move(rhs);
		}

		GoimPacket& operator = (const GoimPacket& rhs)
		{
			if (&rhs != this)
				m_data = rhs.m_data;
			return *this;
		}

		GoimPacket& operator = (GoimPacket&& rhs)
		{
			if (&rhs != this)
				m_data = std::move(rhs.m_data);
			return *this;
		}

		bool SetPayload(const char* jsonData)
		{
			if (!jsonData)
				return false;

			int dataLen = strlen(jsonData);
			return SetPayload(jsonData, dataLen);
		}

		bool SetPayload(const void* data, int dataLen)
		{
			if (data == 0 && dataLen != 0)
				return false;
			char* dstData = GetDataPtrForWrite(dataLen);
			const char* srcData = static_cast<const char*>(data);
			std::copy(srcData, srcData + dataLen, dstData);
			return true;
		}

		const void* GetPayload(int* dataLen) const
		{
			*dataLen = header().get_payload_length();
			return &m_data[header().get_header_length()];
		}

		const char* GetData(int* dataLen) const
		{
			*dataLen = m_data.size();
			return &m_data[0];
		}

		const PacketHeader& GetHeader() const
		{
			return header();
		}

		static GoimPacket ReceivePacket(socket_t s);
	private:
		void InitHeader()
		{
			m_data.resize(sizeof(PacketHeader));
			header().set_packet_length(sizeof(PacketHeader));
			header().set_header_length(sizeof(PacketHeader));
			header().set_protocol_version(1);
		}

		char* GetDataPtrForWrite(uint32_t dataLen)
		{
			int totalLen = header().get_header_length() + dataLen;
			header().set_packet_length(totalLen);
			m_data.resize(totalLen);
			return &m_data[header().get_header_length()];
		}

	private:
		PacketHeader& header()
		{
			return *reinterpret_cast<PacketHeader*>(&m_data[0]);
		}

		const PacketHeader& header() const
		{
			return *reinterpret_cast<const PacketHeader*>(&m_data[0]);
		}

		GoimPacket()
		{
			InitHeader();
		}

		std::vector<char> m_data;
	};

	//接收一整个packet
	GoimPacket GoimPacket::ReceivePacket(socket_t s)
	{
		GoimPacket packet;

		//接收头
		int readByte = ReadSocketFixedByte(s, &packet.header(), sizeof(PacketHeader));

		//检查数据是否正确
		if (readByte != sizeof(PacketHeader))
			throw SocketIOError();

		if (packet.header().get_header_length() != 16)
			throw SocketIOError();

		switch (packet.header().get_command())
		{
		case GoimPacket::RESP_AUTH:
		case GoimPacket::RESP_HEARTBEAT:
		case GoimPacket::RESP_MESSAGE:
			break;
		default:
			throw SocketIOError();
		}

		//会有10M以上的包？出问题了吧
		if (packet.header().get_payload_length() > 10485760)
		{
			//当作是网络错误
			throw SocketIOError();
		}

		int payload_length = packet.header().get_payload_length();
		if (payload_length != 0)
		{
			char* pdata = packet.GetDataPtrForWrite(payload_length);
			int recvLen = ReadSocketFixedByte(s, pdata, payload_length);
			if (recvLen != payload_length)
				throw SocketIOError();
		}

		//到这里都没抛异常那是没问题了
		return std::move(packet);
	}
};

#pragma endregion

BiliDanmakuHime::BiliDanmakuHime(int pRoomId, int pUId)
	: roomId(pRoomId), uId(pUId)
{
}

BiliDanmakuHime::~BiliDanmakuHime()
{
	Stop();
}

//void BiliDanmakuHime::SetOnDisconnectedProc(OnDisconnectedProcT&& proc)
//{
//	onDisconnected = proc;
//}

void BiliDanmakuHime::SetOnJsonProc(OnJsonProcT&& proc)
{
	onJson = proc;
}

void BiliDanmakuHime::SetOnAudienceCountProc(OnAudienceCountProcT&& proc)
{
	onAudienceCount = proc;
}

bool BiliDanmakuHime::Start()
{
	isStopping = false;

	//开线程后台干活
	pthread_create(&networkThread, 0, &BiliDanmakuHime::DanmakuThreadWrapper, this);

	return true;
}

void BiliDanmakuHime::Stop()
{
	if (danmakuSocket != SOCKET_ERROR)
		closesocket(danmakuSocket);
	danmakuSocket = SOCKET_ERROR;

	isStopping = true;

	void* useless;
	pthread_join(networkThread, &useless);
	pthread_detach(networkThread);
}

void* BiliDanmakuHime::DanmakuThreadWrapper(void* p)
{
	BiliDanmakuHime* This = (BiliDanmakuHime*)p;

	while (This->isStopping == false)
	{
		DanmakuThread(p);
		if (This->isStopping)
			break;

		Sleep(BEFORE_RECONNECT_WAITTIME);
	}

	return 0;
}

bool BiliDanmakuHime::DanmakuThread(void* p)
{
	BiliDanmakuHime* This = static_cast<BiliDanmakuHime*>(p);

	int seq = 1;
	try
	{
		//解析域名
		sockaddr_in danmakuServerAddr;
		danmakuServerAddr.sin_family = PF_INET;
		for (int i = 0; i < sizeof(danmakuServerAddr.sin_zero); ++i)
			danmakuServerAddr.sin_zero[i] = 0;
		if (GetHostAddress(&danmakuServerAddr, DANMKAUSERVERADDR) == false)
			return false;

		danmakuServerAddr.sin_port = htons(DANMAKUSERVERPORT);

		//创建套接字
		This->danmakuSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (This->danmakuSocket == SOCKET_ERROR)
			return false;

		//设置到非阻塞
		u_long ioctlParam = 1;
		ioctlsocket(This->danmakuSocket, FIONBIO, &ioctlParam);

		//连上去
		connect(This->danmakuSocket, (SOCKADDR*)&danmakuServerAddr, sizeof(danmakuServerAddr));

		//等待连接成功
		if (WaitSocket<WAITTYPE_WRITE>(This->danmakuSocket) < 0)
		{
			closesocket(This->danmakuSocket);
			return false;
		}

		{//登录请求
			GoimPacket loginPacket(seq++, GoimPacket::REQ_AUTH);
			std::stringstream loginPayload;
			loginPayload << "{\"roomid\":" << This->roomId << ",\"uid\":" << This->uId << "}";
			loginPacket.SetPayload(loginPayload.str().c_str());

			int loginDataLen;
			const char* loginData = loginPacket.GetData(&loginDataLen);

			if (WaitSocket<WAITTYPE_WRITE>(This->danmakuSocket) > 0)
			{
				try
				{
					sendSocketData(This->danmakuSocket, loginData, loginDataLen);
				}
				catch (SocketIOError&)
				{
					closesocket(This->danmakuSocket);
					return false;
				}
			}
			else
			{
				closesocket(This->danmakuSocket);
				return false;
			}
		}

		time_t lastPeopleCountUpdateTime = 0;

		//开始等待和处理推送
		for (;;)
		{
			//是否要更新人数
			//每20秒发送一个心跳指令
			if (time(0) - lastPeopleCountUpdateTime > 20)
			{
				GoimPacket heartBeatPacket(seq++, GoimPacket::REQ_HEARTBEAT);
				int heartBeatPacketLen;
				const char* heartBeatPacketData = heartBeatPacket.GetData(&heartBeatPacketLen);

				sendSocketData(This->danmakuSocket, heartBeatPacketData, heartBeatPacketLen);

				lastPeopleCountUpdateTime = time(0);
			}

			//接收和处理消息
			//如果一段时间没收到就回去检查要不要发心跳
			if (WaitSocket<WAITTYPE_READ>(This->danmakuSocket, 5) > 0)
			{
				GoimPacket recvPacket = GoimPacket::ReceivePacket(This->danmakuSocket);

				switch (recvPacket.GetHeader().get_command())
				{
				case GoimPacket::RESP_AUTH: //验证信息返回，现在这里直接忽略……
					break;

				case GoimPacket::RESP_HEARTBEAT: //心跳数据包，带人数的
				{
					int dataLen;
					const std::uint32_t* pbeAudienceCount;
					pbeAudienceCount = static_cast<const std::uint32_t*>(recvPacket.GetPayload(&dataLen));
					if (dataLen == sizeof(*pbeAudienceCount))
					{
						if (This->onAudienceCount)
							This->onAudienceCount(ntohl(*pbeAudienceCount));
					}
					break;
				}
				case GoimPacket::RESP_MESSAGE: //json数据包，弹幕之类的
				{
					//调用回调函数
					if (This->onJson)
					{
						try
						{
							int dataLen;
							std::vector<char> jsonRawData;
							const char* packetData = static_cast<const char*>(recvPacket.GetPayload(&dataLen));
							jsonRawData.resize(dataLen + 1);
							std::copy(packetData, packetData + dataLen, jsonRawData.begin());
							BiliJsonPtr jsonData(new BiliJson(&jsonRawData[0]));
							This->onJson(std::move(jsonData));
						}
						catch (JsonDataError&)
						{
						}
					}
					break;
				}
				default:
					//???
					break;
				} //switch(packet command)
			}
		} // -> for(;;)
	}
	catch (SocketIOError&)
	{
		closesocket(This->danmakuSocket);
		This->danmakuSocket = SOCKET_ERROR;

		//if (This->onDisconnected)
		//	This->onDisconnected();
		return true;
	}
}
