#include "FFMpegHeader.h"
//��ʱδ��
/// <summary>
/// �����ǹ������򵥷�װ�࣬��˼����Ҫ��ֻ���ƶ�һ�������һ��������м�Ĵ����ɹ�����ָ�������
/// </summary>
class Filter
{
	/// <summary>
	/// ���������
	/// </summary>
	AVFilterContext *buffersink_ctx;
	/// <summary>
	/// ���������
	/// </summary>
	AVFilterContext *buffersrc_ctx;
	/// <summary>
	/// ����������
	/// </summary>
	AVFilterGraph *filter_graph;
public:
	/// <summary>
	///������Ƶ������ѡ��
	/// </summary>
	/// <param name="width">The width.</param>
	/// <param name="height">The height.</param>
	/// <param name="pix_fmt">The pix_fmt.</param>
	/// <param name="timeBase">The time base.</param>
	/// <param name="sampleAspect">The sample aspect.</param>
	/// <param name="outStr">The out string.</param>
	void CreateVideoSrcOptions(int width, int height, AVPixelFormat pix_fmt, AVRational timeBase, AVRational sampleAspect, char* outStr)
	{
		sprintf(outStr,
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			width, height, pix_fmt,timeBase.num, timeBase.den,
			sampleAspect.num, sampleAspect.den);
	}

	/// <summary>
	/// ������Ƶ������ѡ��
	/// </summary>
	/// <param name="sample_fmt">The sample_fmt.</param>
	/// <param name="sample_rate">The sample_rate.</param>
	/// <param name="channel_layout">The channel_layout.</param>
	/// <param name="timeBase">The time base.</param>
	/// <param name="outStr">The out string.</param>
	void CreateAudioSrcOptions(AVSampleFormat sample_fmt, int sample_rate, uint64_t channel_layout, AVRational timeBase, char* outStr)
	{
		sprintf(outStr,
			"sample_fmt=%s:time_base=%d/%d:sample_rate=%d:channel_layout=0x%x",
			av_get_sample_fmt_name(sample_fmt), timeBase.num, timeBase.den, sample_rate, channel_layout);
	}
	void CreateAudioSrcOptions1(AVSampleFormat sample_fmt, int sample_rate, uint64_t channel_layout, AVRational timeBase, char* outStr)
	{
		sprintf(outStr,
			"sample_fmt=%s:time_base=%d/%d:sample_rate=%d:channel_layout=mono",
			av_get_sample_fmt_name(sample_fmt), timeBase.num, timeBase.den, sample_rate);
	}

	/// <summary>
	/// ������Ƶת��ʽ������ѡ��
	/// </summary>
	/// <param name="sample_fmt">The sample_fmt.</param>
	/// <param name="sample_rate">The sample_rate.</param>
	/// <param name="channel_layout">The channel_layout.</param>
	/// <param name="outStr">The out string.</param>
	void aformatOptions(AVSampleFormat sample_fmt, int sample_rate, uint64_t channel_layout, char* outStr)
	{
		sprintf(outStr,
			"aformat=sample_fmts=%s:sample_rates=%d:channel_layouts=0x%x",
			av_get_sample_fmt_name(sample_fmt), sample_rate,
			(uint64_t)channel_layout);
	}
	void aformatOptions1(AVSampleFormat sample_fmt, int sample_rate, uint64_t channel_layout, char* outStr)
	{
		sprintf(outStr,
			"aformat=sample_fmts=%s:sample_rates=%d:channel_layouts=stereo",
			av_get_sample_fmt_name(sample_fmt), sample_rate);
	}
	//pad=1280:800:0:40:black 
	/// <summary>
	/// ��ʼ��������
	/// </summary>
	/// <param name="mediaType">��Ƶ������Ƶ</param>
	/// <param name="srcOptions">���������ѡ��</param>
	/// <param name="filterDescr">���������ѡ��</param>
	void Initial(AVMediaType mediaType, const char* srcOptions, const char * filterDescr)
	{
		AVFilter *buffersrc = NULL;
		AVFilter *buffersink = NULL;
		switch (mediaType)
		{
		case AVMEDIA_TYPE_VIDEO:
			buffersrc = avfilter_get_by_name("buffer");
			buffersink = avfilter_get_by_name("buffersink");
			break;
		case AVMEDIA_TYPE_AUDIO:
			buffersrc = avfilter_get_by_name("abuffer");
			buffersink = avfilter_get_by_name("abuffersink");
			break;
		default:
			throw;
		}

		int ret;
		AVFilterInOut *outputs = avfilter_inout_alloc();
		AVFilterInOut *inputs = avfilter_inout_alloc();
		filter_graph = avfilter_graph_alloc();
		if (!outputs || !inputs || !filter_graph) {
			
			throw "dd";
		}
		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", srcOptions, NULL, filter_graph);
		if (ret < 0) {
			printf("Cannot create buffer source\n");
		}

	
		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, NULL, filter_graph);
		if (ret < 0) {
			printf("Cannot create buffer sink\n");
		}

		outputs->name = av_strdup("in");
		outputs->filter_ctx = buffersrc_ctx;
		outputs->pad_idx = 0;
		outputs->next = NULL;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = buffersink_ctx;
		inputs->pad_idx = 0;
		inputs->next = NULL;

		if ((ret = avfilter_graph_parse_ptr(filter_graph, filterDescr, &inputs, &outputs, NULL)) < 0)
		{
			printf("avfilter_graph_parse_ptr\n");
			throw "ff";
		}

		if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		{
			printf("avfilter_graph_config");
			throw "kk";
		}
			

		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="Filter"/> class.
	/// </summary>
	~Filter()
	{
		avfilter_graph_free(&filter_graph);
	}

	/// <summary>
	/// ѹ������
	/// </summary>
	/// <param name="frame">The frame.</param>
	/// <returns></returns>
	int Push(AVFrame* frame)
	{
		int  ret = av_buffersrc_add_frame(buffersrc_ctx, frame);
		return ret;
	}

	/// <summary>
	/// ��ȡ����
	/// </summary>
	/// <param name="frame">The frame.</param>
	/// <returns></returns>
	int Get(AVFrame* frame)
	{
		int  ret = av_buffersink_get_frame_flags(buffersink_ctx, frame, 0);
		 return ret;
	}
};

