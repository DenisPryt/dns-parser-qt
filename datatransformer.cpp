#include "datatransformer.h"

#include <QList>
#include <QDataStream>
#include <exception>

DnsDataStream::DnsDataStream(QByteArray *dnsPacket)
{
    m_position  = 0;
    m_dnsPacket = dnsPacket;
}

DnsDataStream &DnsDataStream::domainName(QString &domain)
{
    int endOfPos = m_position;
    domain = fromDomainToString( *m_dnsPacket, m_position, &endOfPos );
    m_position = endOfPos;

    return *this;
}

DnsDataStream &DnsDataStream::domainName(QByteArray &domain)
{
    int endOfPos = m_position;
    domain = fromDomainToBytes( *m_dnsPacket, m_position, &endOfPos );
    m_position = endOfPos;

    return *this;
}

DnsDataStream &DnsDataStream::number16(quint16 &number)
{
    number = (m_dnsPacket->at( m_position ) << 8) | m_dnsPacket->at( m_position + 1 );
    m_position += sizeof(quint16);

    return *this;
}

DnsDataStream &DnsDataStream::number32(quint32 &number)
{
    number = (m_dnsPacket->at( m_position + 0 ) << 24) | m_dnsPacket->at( m_position + 1 ) << 16
           | (m_dnsPacket->at( m_position + 2 ) << 8 ) | m_dnsPacket->at( m_position + 3 );
    m_position += sizeof( quint32 );

    return *this;
}

DnsDataStream &DnsDataStream::question(DnsQuestion &question)
{
    QByteArray domian;
    quint16 type, cls;
    domainName( domian ).number16( type ).number16( cls );
    question = DnsQuestion( domian, (RRTypes::RRType)type, (RRClasses::RRClass)cls );
    return *this;
}

DnsDataStream &DnsDataStream::resourceData(QByteArray &resData )
{
    quint16 len = 0;
    number16( len );
    resData = m_dnsPacket->mid( m_position, len );
    Q_ASSERT( resData.size() == len );
    m_position += len;
    return *this;
}

DnsDataStream &DnsDataStream::resourceRecord(DnsResourceRecord &resRec)
{
    quint32 ttl = 0;
    QByteArray resourceData;
    question( resRec ).number32( ttl ).resourceData( resourceData );

    resRec.setTtl( ttl );
    resRec.setResourceData( std::move(resourceData) );

    return *this;
}

QByteArray DnsDataStream::transformDomain(const QByteArray &domain)
{
    QByteArray result;

    QList<QByteArray> marksList = domain.split('.');
    for ( const QByteArray &mark : marksList ){
        char markSize = mark.size();
        result.append( markSize ).append( mark );
    }

    if ( !domain.endsWith('.') ){
        result.append( (char)0 );
    }

    return result;
}

QByteArray DnsDataStream::transformDomain(const QString &domain )
{
    return transformDomain( domain.toUtf8() );
}

QByteArray DnsDataStream::fromDomainToBytes(const QByteArray &domain, int *posOfEnd)
{
    QByteArray result;

    int i = 0;
    for ( i = 0; i < domain.size(); ++i ){
        int markLen = domain[ i ];
        if ( markLen == 0 ){
            ++i;
            break;
        }

        result.append( domain.mid( i + 1, markLen ) ).append( '.' );
        i += markLen;
    }

    if ( posOfEnd != nullptr )
        *posOfEnd = i;

    return result;
}

QString DnsDataStream::fromDomainToString(const QByteArray &domain, int *posOfEnd)
{
    return QString::fromUtf8( fromDomainToBytes(domain, posOfEnd) );
}

QByteArray DnsDataStream::fromDomainToBytes(const QByteArray &packet, int start, int *posOfEnd )
{
    QByteArray domain;

    quint16 indexFromStart = (uchar)packet.at( start );
    if ( indexFromStart >= 192 ){
        indexFromStart &= ~192;  // 192 = 11000000
        indexFromStart  = indexFromStart << 8;
        indexFromStart |= (uchar)packet.at( start + 1 );

        int end = packet.indexOf( (char)0, indexFromStart );
        if ( end < 0 ){
            return domain;
        }
        if ( posOfEnd != nullptr ){
            *posOfEnd = start + sizeof(quint16);
        }
        domain = packet.mid( indexFromStart, end - indexFromStart + 1 );
        return fromDomainToBytes( domain );
    }
    else{
        domain = packet.mid( start );
        auto res = fromDomainToBytes( domain, posOfEnd );
        if ( posOfEnd != nullptr ){
            *posOfEnd += start;
        }
        return res;
    }
}

QString DnsDataStream::fromDomainToString(const QByteArray &packet, int start, int *posOfEnd)
{
    return QString::fromUtf8( fromDomainToBytes(packet, start, posOfEnd) );
}
