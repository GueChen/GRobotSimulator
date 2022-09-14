#include "skinsystem.h"
#include "GComponent/GTransform.hpp"
#include "function/conversion.hpp"

using std::vector;
using Eigen::Vector3f;
using Eigen::Matrix3f;

namespace GComponent {

	SkinSystem& SkinSystem::getInstance()
	{
		static SkinSystem instance;
		return instance;
	}

	SkinSystem::~SkinSystem() = default;

	void SkinSystem::Initialize() {
		Portname();
	}


    /*_____________________________________SLOTS METHODS_______________________*/
    void SkinSystem::Run()
    {
        int baudrate = 9600;
        serial = new QSerialPort;

        serial->setPortName(_PortName);
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(baudrate);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        QObject::connect(serial, &QSerialPort::readyRead, this, &SkinSystem::ReadData2);

    }

    void SkinSystem::Close()
    {
        serial->clear();
        serial->close();
        serial->deleteLater();
    }

    void SkinSystem::GetPort(QString port)
    {
        _PortName = port;
    }

    void SkinSystem::GetBase()
    {
        int sam_num = 20;
        for (int i = 0; i < sam_num; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                sum[j] = sum[j] + _Vfloat[j];
            }
        }        
        for (int i = 0; i < 8; i++)
        {
            basevalue[i] = (sum[i] / sam_num);
        }
    }

    void SkinSystem::ClearBase()
    {
        basevalue = { 0,0,0,0,0,0,0,0 };
        sum = { 0,0,0,0,0,0,0,0 };
    }

    void SkinSystem::ReadPortName()
    {
        _Nlist.clear();
        Portname();
        emit SendPortList(_Nlist);
    }

    void SkinSystem::SetSensorVector(int unitname, int count, std::vector<float> tar_dir)
    {
        /*enum SpaceType : int {
            unit1 = 0, unit2, unit3, unit4, unit5, unit6, unit7, unit8,
        }target_unit = static_cast<SpaceType>(unitname);*/
        //Vector3f Axis_Z = {0,0,1};
        _UnitName = unitname;
        _CountNum = count;

        _VectorList = vector<Vector3f>(count);
        _RotVector = vector<Vector3f>(count-1);
        _RotMatrix = vector<Matrix3f>(count-1);

        _VectorList[0] = STLUtils::toVec3f(tar_dir).normalized();
        qDebug() << _VectorList[0][0] << _VectorList[0][1] << _VectorList[0][2];

        for (int i = 0; i < _CountNum - 1; ++i)
        {
            _RotVector[i] = { 0, 0, 2 * (i + 1) * (float)M_PI / count };
            _RotMatrix[i] = Roderigues(_RotVector[i]);
            _VectorList[i + 1] = _RotMatrix[i] * _VectorList[0];
            qDebug() << _VectorList[i + 1][0]<< _VectorList[i + 1][1]<<_VectorList[i + 1][2];
        }
    }

    /*_____________________________________PRIVATE_____________________________*/
    void SkinSystem::Portname()
    {
        foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
        {
            if (!info.isNull()) {
                _Nlist << info.portName();
            }
        }
    }

    void SkinSystem::ReadData2()
    {
        QByteArray buf;
        uchar flag;
        float h1, l1, l2, l3;
        float datavalue;
        float newvalue;

        buf = serial->readAll();
        for (int i = 0; i < buf.size(); i++)
        {
            databuf.enqueue(buf.at(i));
        }
        buf.clear();

        if (databuf.size() > 64)
        {
            DataProcess(flag, h1, l1, l2, l3, datavalue, newvalue);
            emit SendValue(_Vfloat);
            //emit SendString(_Str);
            //emit SendTrigger(Trigger);
        }
    }    

    void SkinSystem::DataProcess(uchar flag,
                                float h1, float l1, float l2, float l3,
                                float datavalue, float newvalue)
    {
        do {
            flag = databuf.dequeue();
        } while (flag != 0xff);
        for (int i = 0; i < 16; i++)
        {
            value[i] = databuf.dequeue();
        }
        for (int i = 0; i < 8; i++)
        {
            h1 = (value[2 * i] & 0xf0) >> 4;
            l1 = value[2 * i] & 0x0f;
            l2 = (value[2 * i + 1] & 0xf0) >> 4;
            l3 = value[2 * i + 1] & 0x0f;

            datavalue = (float)(h1 + l1 / 10 + l2 / 100 + l3 / 1000);
            newvalue = LPFilter(prevalue[i], datavalue);
            ProximityTrigger(newvalue, basevalue[i], i);

            prevalue[i] = newvalue;
            newvalue = ((newvalue - basevalue[i]) * (4 / (4 - basevalue[i])));
            _Str[i] = QString::number(newvalue, 'f', 3);
            _Vfloat[i] = _Str[i].toFloat();
        }
    }

    float SkinSystem::LPFilter(float predata, float curdata)
    {
        float a = 0.112;
        float newdata;
        newdata = a * curdata + (1 - a) * predata;
        return newdata;
    }

    void SkinSystem::ProximityTrigger(float currentV, float baseV, int i)
    {
        float _Threshold = 0.3;
        mutex.lock();
        if (baseV != 0)
        {            
            currentV = ((currentV - baseV) * (4 / (4 - baseV)));
            currentV > _Threshold ? _Trigger[i] = 1 : _Trigger[i] = 0;
        }
        mutex.unlock();
    }
 
    /*_____________________________________PUBLIC_____________________________*/
    vector<Vector3f> SkinSystem::GetTriVector()
    {
        vector<Vector3f> TriggerVector;
        for (int i = _UnitName; i < (_UnitName + _CountNum) ; ++i)
        {
            if (_Trigger[i] == 1)
            {
                TriggerVector.push_back(_VectorList[i - _UnitName]);
            }
        }        
        return TriggerVector;
    }

    bool SkinSystem::Flag()
    {
        return _Trigger.contains(1);
    }
}