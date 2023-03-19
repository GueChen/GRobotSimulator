/**
* 
* 
* @author Gwz
* 
**/
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "base/singleton.h"

#include <QtCore/QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCore/QQueue>
#include <QtCore/QMutex>
#include <QtCore/QList>
#include <QtCore/QDebug>

#include <Eigen/Core>

using std::vector;
using Eigen::Vector3f;
using Eigen::Matrix3f;

namespace GComponent {

class SkinSystem : public QObject
{
    Q_OBJECT

    NonCopyable(SkinSystem)

public:
    static SkinSystem&  getInstance();
    ~SkinSystem();
    void                Initialize();
    bool                Flag();
    vector<Vector3f>    GetTriVector();
                  
private:   
    void                Portname();
    void                ReadData2();        
    void                DataProcess(uchar flag,
                                float h1, float l1, float l2, float l3,
                                float datavalue, float newvalue);
    float               LPFilter(float predata, float curdata);
    void                ProximityTrigger(float currentV, float baseV, int i);        


public slots:
    void                Run();
    void                Close();
    void                GetPort(QString port);
    void                GetBase();
    void                ClearBase();
    void                ReadPortName();
    void                SetSensorVector(int unitname, int count, std::vector<float> tar_dir);
    void                SetUsingMask(int idx, bool flag);

signals:
    void                SendValue(QList<float> v);
    void                SendString(QList<QString> str);
    void                SendPortList(QList<QString> namelist);
    //void SendTrigger(QList<float> Trigger);

protected:
    SkinSystem() = default;

public:    
    QList<float>        _Vfloat  = QList<float>(8);
    QList<QString>      _Str     = QList<QString>(8);
    QList<QString>      _Nlist;
    QList<float>        _Trigger = { 0,0,0,0,0,0,0,0 };

private:
    QSerialPort*        serial;
    QString             _PortName;
    QQueue<uchar>       databuf;
    QList<uchar>        value     = QList<uchar>(16);
    QList<float>        prevalue  = { 0,0,0,0,0,0,0,0 };
    QList<float>        sum       = { 0,0,0,0,0,0,0,0 };
    QList<float>        basevalue = { 0,0,0,0,0,0,0,0 };
    QMutex              mutex;

    vector<Vector3f>    _VectorList;
    vector<Vector3f>    _RotVector;
    vector<Matrix3f>    _RotMatrix;
    int                 _UnitName;
    int                 _CountNum;
    int                 using_mask_ = (1 << 8) - 1; //0xff
};

}
#endif