#ifndef VOICECODEC_H
#define VOICECODEC_H

#include <Q3ValueList>
#include <qstring.h>

class VoiceEncoder {
public:
	VoiceEncoder();
	virtual ~VoiceEncoder();

    // encode chunk of audio data
    // return value: the size of *res, 0 is ok, becouse of prebuffering
    // samplesProcessed: number of samples, coded used to encode res,
    // this is needed for RTP timing
    virtual int encode( const short *data, int size, char **res,int *samplesPocessed ) = 0;

};

class VoiceDecoder {
public:
	VoiceDecoder();
	virtual ~VoiceDecoder();

    // decode chunk of audio data
    // return value: the size of *res, 0 is ok, becouse of prebuffering
    virtual int decode( const char *data, int size, short **res ) = 0;

};


class VoiceCodecFactory {
public:
    //VoiceCodecFactory();
	//virtual ~VoiceCodecFactory() {};
    
    // codec's description 
    virtual QString name() = 0;
    virtual QString description() { return ""; }
    virtual double bandwidth() = 0;

    // RTP payload type, rfc1890
    virtual int payload() = 0;

    // a:rtmap SDP argument, like "SPEEX/8000" 
    virtual QString rtmap() = 0;

    virtual VoiceEncoder *encoder() = 0;
    virtual VoiceDecoder *decoder() = 0;
};


class CodecsManager {
public:
    CodecsManager();
    ~CodecsManager();

    static CodecsManager * instance();

    void installCodecFactory( VoiceCodecFactory* );

    VoiceCodecFactory* codecFactory( int payload );
    //VoiceCodecFactory* codecFactory( QString rtmap );

// Changet to provide QT4 support     QValueList<int> payloads();
    Q3ValueList<int> payloads();

private:    
    class Private;
    Private *d;
};

#endif // VOICECODEC_H

