#ifndef KIT_H
#define KIT_H

#include <QByteArray>
#include <QString>


unsigned short CRC16(const QByteArray& byte_array, int len); // CRC计算
QString convertHex2Str(QByteArray hex);
QString convertHexChar2Str(char *hex, int len);


#endif // KIT_H
