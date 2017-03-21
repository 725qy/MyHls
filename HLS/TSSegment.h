#pragma once
#include <string>
#include<vector>
#include<deque>
#include<mutex>
using namespace std;
//TS��
struct TsSeg
{
	//ʱ��,����Ϊ��λ��С���㱣��6λ
	float duration;
	string name;
	vector<unsigned char> data;
};

// һ��session�����͵�TS�黺��
struct TsSegBuffer
{
	//Ts�����
	deque<TsSeg> segs;
	//�б�˳�򣬱�ʾ#EXT-X-MEDIA-SEQUENCE��������������ʾ�б����
	size_t sequence = 0;
	//�Ƿ񻬶�,�б�䶯�ֳ����֣�һ���ǵ�����һ���ǻ���
	bool isSlide;
	//����push��������
	size_t pushCount = 0;
	//����TS����Ŀ
	int segCountMax = 0;
	//���п����ʱ��,��m3u8��#EXT-X-TARGETDURATION
	float targetDuration;
	mutex segsLocker;
public:
	TsSegBuffer& operator =(const TsSegBuffer &buffer)
	{
		if (this == &buffer) {
			return *this;
		}
		this->segs = buffer.segs;
		sequence = 0;
		isSlide = false;
		pushCount = 0;
		targetDuration = 0;
		for each (auto seg in segs)
		{
			if (targetDuration < seg.duration)
			{
				targetDuration = seg.duration;
			}
		}
		return *this;
	}

	/// <summary>
	/// Sets the seg count maximum.
	/// </summary>
	/// <param name="segCountMax">The seg count maximum.</param>
	void SetSegCountMax(int segCountMax)
	{
		this->segCountMax = segCountMax;
	}

	/// <summary>
	/// Pushes the specified duration.
	/// </summary>
	/// <param name="duration">The duration.</param>
	/// <param name="index">The index.</param>
	/// <param name="data">The data.</param>
	void Push(float duration, int index, vector<unsigned char>& data)
	{
		char tmp[50] = { 0 };
		sprintf_s(tmp, "%d.ts", index);
		TsSeg seg{ duration, tmp, data };
		if (segs.size() >= segCountMax)
		{
			segsLocker.lock();
			segs.pop_front();
			segsLocker.unlock();
			isSlide = true;
		}
		segsLocker.lock();
		segs.push_back(seg);
		segsLocker.unlock();
		if (targetDuration < seg.duration)
		{
			targetDuration = seg.duration;
		}
		pushCount++;
	}

	/// <summary>
	/// Gets the m3u8.
	/// </summary>
	/// <returns></returns>
	string GetM3u8()
	{
		if (isSlide)
		{
			sequence++;
			isSlide = false;
		}
		char str[200] = { 0 };
		if (pushCount == segCountMax || pushCount == 1)
		{
			sprintf_s(str, "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-ALLOW-CACHE:NO\n#EXT-X-MEDIA-SEQUENCE:%ld\n#EXT-X-TARGETDURATION:%d\n", sequence, (int)(targetDuration + 1));
			//sprintf_s(str, "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:%ld\n#EXT-X-TARGETDURATION:20\n#EXT-X-DISCONTINUITY\n", sequence);
		}
		else
		{
			sprintf_s(str, "#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-ALLOW-CACHE:NO\n#EXT-X-MEDIA-SEQUENCE:%ld\n#EXT-X-TARGETDURATION:%d\n", sequence, (int)(targetDuration + 1));
		}


		string ret = str;
		segsLocker.lock();
		for each (auto seg in segs)
		{
			if (!seg.name.empty())
			{
				memset(str, 0, 100);
				sprintf_s(str, "#EXTINF:%.6f,\n%s\n", seg.duration, seg.name.c_str());
				ret += str;
			}
		}
		segsLocker.unlock();
		return ret;
	}

	/// <summary>
	/// Gets the ts.
	/// </summary>
	/// <param name="name">The name.</param>
	/// <returns></returns>
	vector<unsigned char> GetTs(const char* name)
	{
		for (auto seg = segs.begin(); seg != segs.end();)
		{
			if (seg->name == name)
			{
				return seg->data;
			}
			else
				seg++;
		}
		vector<unsigned char> temp;
		return temp;
	}
};