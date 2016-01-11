#include "dnspacket.h"
#include <QUdpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QUdpSocket  udp;

    QList< QString > requests;
    for ( int i = 1; i < argc; ++i ){
        requests << QString::fromUtf8( argv[i], strlen( argv[i] ) );
    }
    if ( argc == 1 ){
        //std::cout << "Enter domain names" << std::endl;
        //std::string domain;
        //std::getline( std::cin, domain );
        //requests << QString::fromStdString( domain ).split(' ', QString::SkipEmptyParts );
    }

    DnsPacket requestPacket;
    requestPacket.addQuestion( DnsQuestion::getName("119.131.240.87") );
    requestPacket.addQuestion( DnsQuestion::getMailExchanger("mail.ru") );
    udp.writeDatagram( requestPacket.toByteArray(), QHostAddress("8.8.8.8"), 53 );

    udp.waitForReadyRead();
    char data[512]   = {0};
    int  size        = udp.readDatagram( data, 512 );
    DnsPacket packet = DnsPacket::fromBytesArary( QByteArray( data, size ) );

    for ( auto & q : packet.questions() )
        qDebug() << q.domainName() << q.type() << q.reqClass();
    for ( DnsResourceRecord a : packet.answers() )
        qDebug() << a.domainName() << a.toString();

}
