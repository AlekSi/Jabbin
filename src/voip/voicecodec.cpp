#include "voicecodec.h"

#include <Q3ValueList>
#include <Q3IntDict>
#include <QDebug>

#include "codecs/pcmucodec.h"
#include "codecs/speexcodec.h"

VoiceEncoder::VoiceEncoder(){}
VoiceEncoder::~VoiceEncoder(){}
VoiceDecoder::VoiceDecoder(){}
VoiceDecoder::~VoiceDecoder(){}



CodecsManager * codecsManagerInstance = 0;

class CodecsManager::Private {
public:
    Q3IntDict<VoiceCodecFactory> codecs; // payload->VoiceCodecFactory dictionary
    //QDict<VoiceCodecFactory> rtmapDicts; // payload->VoiceCodecFactory dictionary
    Q3ValueList<int> payloads;
};

CodecsManager::CodecsManager()
{
    d = new Private();
    codecsManagerInstance = this;

    qDebug() << "CodecsManager::CodecsManager() installing codecs";
    installCodecFactory( new PCMUCodecFactory() );
//    installCodecFactory( new SpeexCodecFactory() );
}

CodecsManager::~CodecsManager()
{
    delete d;
}


CodecsManager * CodecsManager::instance()
{
    if ( !codecsManagerInstance )
        return new CodecsManager();

    return codecsManagerInstance;
}

void CodecsManager::installCodecFactory( VoiceCodecFactory* factory )
{
    d->codecs.insert( factory->payload(), factory );
    //d->rtmapDicts.insert( factory->rtmap(), factory );
    d->payloads.append( factory->payload() );
}

VoiceCodecFactory* CodecsManager::codecFactory( int payload )
{
    return d->codecs[payload];
}

/*
VoiceCodecFactory* CodecsManager::codecFactory( QString rtmap )
{
    return d->rtmapDicts[rtmap];
}*/


Q3ValueList<int> CodecsManager::payloads()
{
    return d->payloads;
}

