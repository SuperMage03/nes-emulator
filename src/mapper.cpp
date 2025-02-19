#include "mapper.hpp"

std::unique_ptr<Mapper> Mapper::makeMapper(const uint8_t& mapper_id) {
    switch (mapper_id) {
        default: {
            return nullptr;
        }
    }
}
