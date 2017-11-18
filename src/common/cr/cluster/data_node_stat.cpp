#include "data_node_stat.h"

namespace cr
{
    namespace cluster
    {
        DataNodeStat::DataNodeStat()
            : version_(0),
            cversion_(0),
            dversion_(0)
        {}

        DataNodeStat::~DataNodeStat()
        {}

        void DataNodeStat::setVersion(std::uint64_t version)
        {
            version_ = version;
        }

        std::uint64_t DataNodeStat::getVersion() const
        {
            return version_;
        }

        void DataNodeStat::setCversion(std::uint64_t cversion)
        {
            cversion_ = version_;
        }

        std::uint64_t DataNodeStat::getCversion() const
        {
            return cversion_;
        }

        void DataNodeStat::setDversion(std::uint64_t dversion)
        {
            dversion_ = dversion;
        }

        std::uint64_t DataNodeStat::getDversion() const
        {
            return dversion_;
        }

        void DataNodeStat::setOwner(std::string owner)
        {
            owner_ = std::move(owner);
        }

        const std::string& DataNodeStat::getOwner() const
        {
            return owner_;
        }
    }
}