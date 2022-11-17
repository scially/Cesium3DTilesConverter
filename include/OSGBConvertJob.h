#pragma once

#include <OSGBLevel.h>
#include <TilesConvertException.h>

#include <QRunnable>
#include <QThreadPool>
#include <QVector>
#include <QtDebug>

namespace scially {
    class OSGBConvertTask;

    class OSGBConvertJob: public QObject{
        Q_OBJECT
    public:
        OSGBConvertJob(const QString& input, const QString &output, QObject* parent = nullptr)
            :input_(input), output_(output), QObject(parent) {
            threadPool_ = new QThreadPool(this);
        }

        void setThread(int thread){
            threadPool_->setMaxThreadCount(thread);
        }

        void run();

        void setHeight(double h) {height_ = h; }
        void setMaxLevel(int level){ maxLevel_ = level; }
        void setYUpAxis(bool yUpAxis){yUpAxis_ = yUpAxis; }

    private:
        QString input_;
        QString output_;
        double height_ = 0;
        bool yUpAxis_ = false;
        QThreadPool * threadPool_;
        int maxLevel_ = std::numeric_limits<int>::max();
        QVector<OSGBConvertTask*> tasks_;
    };

    class OSGBConvertTask: public QObject, public QRunnable{
        Q_OBJECT
    public:
        OSGBConvertTask(const QString& input, const QString& output, 
                        int maxLevel = OSGLevel::MAXLEVEL, QObject* parent = nullptr)
            :osgbLevel_(input), output_(output), maxLevel_(maxLevel), QObject(parent){
            setAutoDelete(false);
        };

        virtual void run() override {
            qInfo() << "start process tile:" << osgbLevel_.nodeName();

            success_ = osgbLevel_.convertTiles(tile_, output_, maxLevel_);
            if (success_)
                qInfo() << "finish process tile:" << osgbLevel_.nodeName();
            else
                qCritical() << "error process tile" << osgbLevel_.nodeName();
        }
        
        void setYUpAxis(bool y) noexcept { osgbLevel_.setYUpAxis(y); }

        OSGBRegion region() const noexcept { return osgbLevel_.region(); }
        bool isSuccess() const { return success_; }
        BaseTile baseTile() const { return tile_; }
        QString tileName() const { return osgbLevel_.getTileName(); }
    private:
        OSGLevel osgbLevel_;
        bool success_ = false;
        BaseTile tile_;
        QString output_;
        int maxLevel_;
    };
}