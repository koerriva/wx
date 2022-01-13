//
// Created by koerriva on 2022/1/13.
//

#include "systems.h"

namespace wx {
    vec3 lerp(vec3 prev,vec3 next,float interp){
        return prev+interp*(next-prev);
    }

    void animator_update_system(level* level,float delta){
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Animator>(level,entity)){
                auto animator = level_get_component<Animator>(level,entity);
                size_t animation_count = animator->animations.size();
                if(animator->state==Animator::pause){}
                if(animator->state==Animator::play){
                    if(animator->playState==Animator::begin){
                        for (int i = 0; i < animation_count; ++i) {
                            animation_t* animation = &animator->animations[i];
                            for (int j = 0; j < animation->channel_count; ++j) {
                                channel_t * channel = animation->channels+j;
                                auto target_transform = level_get_component<Transform>(level,channel->target);
                                if(!level_has_components<AnimatedTransform>(level,channel->target)){
                                    AnimatedTransform animatedTransform = AnimatedTransform::From(target_transform);
                                    level_add_component<AnimatedTransform>(level,channel->target,animatedTransform);
                                }
                            }
                        }

                        animator->playState = Animator::playing;
                    }else if(animator->playState==Animator::playing){
                        //update
                        for (int i = 0; i < animation_count; ++i) {
                            animation_t * animation = &animator->animations[i];

                            bool isPlayThisAnimation;
                            isPlayThisAnimation = strcmp(animator->playingAnimation.c_str(),animation->name)==0;
                            isPlayThisAnimation = isPlayThisAnimation || strcmp(animator->playingAnimation.c_str(), "") == 0 && i == 0;

                            if(!isPlayThisAnimation){
                                continue;
                            }

                            int finished_channel = 0;

                            for (int j = 0; j < animation->channel_count; ++j) {
                                channel_t* channel = animation->channels+j;

                                keyframe_t* base_keyframe = channel->keyframe;
                                int keyframe_count = channel->keyframe_count;

                                for (int k = 0; k < keyframe_count; ++k) {
                                    keyframe_t* keyframe = base_keyframe+k;
                                    if(keyframe->time<=animator->currTime[i]){
                                        animator->prevFrame[i][j] = keyframe;
                                    }
                                }

                                if(animator->prevFrame[i][j] == nullptr){
                                    animator->prevFrame[i][j] = new keyframe_t ;
                                }

                                bool lastKeyframe = true;
                                for (int k = 0; k < keyframe_count; ++k) {
                                    keyframe_t* keyframe = base_keyframe+k;
                                    if(keyframe->time>animator->currTime[i]){
                                        animator->nextFrame[i][j] = keyframe;
                                        lastKeyframe = false;
                                        break;
                                    }
                                }

                                if(lastKeyframe){
                                    finished_channel++;
                                    continue;
                                }

                                //linear
                                float interp = (animator->currTime[i] - animator->prevFrame[i][j]->time)/(animator->nextFrame[i][j]->time - animator->prevFrame[i][j]->time);

                                auto origin_transform = level_get_component<Transform>(level,channel->target);
                                auto animated_transform = level_get_component<AnimatedTransform>(level,channel->target);
                                bool jointed = level_has_components<Joint>(level,channel->target);

                                if(channel->has_translation){
                                    vec3 prev = animator->prevFrame[i][j]->translation;
                                    vec3 next = animator->nextFrame[i][j]->translation;
                                    vec3 translation = lerp(prev,next,interp);//lerp
                                    animated_transform->position = origin_transform->position + translation;
                                    if(jointed){
                                        animated_transform->position = translation;
                                    }
                                }

                                if(channel->has_rotation){
                                    quat prev = animator->prevFrame[i][j]->rotation;
                                    quat next = animator->nextFrame[i][j]->rotation;
                                    quat rotation = slerp(prev, next,interp);
                                    animated_transform->rotation = origin_transform->rotation * rotation;
                                    if(jointed){
                                        animated_transform->rotation = rotation;
                                    }
                                }

                                if(channel->has_scale){
                                    vec3 prev = animator->prevFrame[i][j]->scale;
                                    vec3 next = animator->nextFrame[i][j]->scale;
                                    vec3 scale = lerp(prev,next,interp);//lerp
                                    animated_transform->scale = origin_transform->scale * scale;
                                    if(jointed){
                                        animated_transform->scale = scale;
                                    }
                                }
                            }

                            if(finished_channel>=animation->channel_count){
                                animator->currTime[i] = 0.0;
                                for (int j = 0; j < animation->channel_count; ++j){
                                    animator->prevFrame[i][j] = nullptr;
                                    animator->nextFrame[i][j] = nullptr;
                                }
                                continue;
                            }

                            animator->currTime[i] += delta;
                        }
                    }else if(animator->playState==Animator::end){
                    }
                }
                if(animator->state==Animator::stop){
                    //stop
                    for (int i = 0; i < animation_count; ++i) {
                        animator->currTime[i] = 0;//时间同步
                        animation_t* animation = &animator->animations[i];
                        for (int j = 0; j < animation->channel_count; ++j) {
                            animator->prevFrame[i][j] = nullptr;
                            animator->nextFrame[i][j] = nullptr;

                            channel_t * channel = animation->channels+j;
                            if(level_has_components<AnimatedTransform>(level,channel->target)){
                                level_remove_component<AnimatedTransform>(level,channel->target);
                            }
                        }
                    }
                }
            }
            entities_iter++;
        }
    }
}