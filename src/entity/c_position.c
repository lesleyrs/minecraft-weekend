#include "c_position.h"
#include "ecs.h"
#include "../world/world.h"

static void tick(struct PositionComponent *c_position, struct Entity entity) {
    ivec3s block = world_pos_to_block(c_position->position);
    ivec3s offset = world_pos_to_offset(block);

    if (!glms_ivec3_eqv(block, c_position->block)) {
        c_position->block = block;
        c_position->block_changed = true;
    } else {
        c_position->block_changed = false;
    }

    if (!glms_ivec3_eqv(offset, c_position->offset)) {
        c_position->offset = offset;
        c_position->offset_changed = true;
    } else {
        c_position->offset_changed = false;
    }
}

void c_position_init(struct ECS *ecs) {
    ecs_register(C_POSITION, struct PositionComponent, ecs, ((union ECSSystem) {
        .init = NULL,
        .destroy = NULL,
        .render = NULL,
        .update = NULL,
        .tick = (ECSSubscriber) tick
    }));
}
