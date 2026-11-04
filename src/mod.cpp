#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
std::uint16_t previous=0;
bool restart_requested=false;
bool was_alive=true;
int deaths=0;
bool show_counter=true;
bool skip_death_scenes=true;
int skip_after=45;
}
void load(openrac::Mod& mod) {
    if(mod.game()!="rac1" || !mod.has_play()) throw openrac::Error("Death Improvements requires RAC1 play API");
    g=&mod;
    show_counter=mod.setting<bool>("show_counter");
    skip_death_scenes=mod.setting<bool>("skip_death_scenes");
    skip_after=static_cast<int>(mod.setting<std::int64_t>("skip_after_frames"));
    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto mask=static_cast<std::uint16_t>(OPENRAC_PAD_L1|OPENRAC_PAD_R1|OPENRAC_PAD_CIRCLE);
        if(qol::combo_pressed(now,previous,mask)) restart_requested=true;
        if((now&mask)==mask) qol::consume_buttons(report,mask);
        previous=now;
    });
    mod.on_frame([]{
        const auto status=g->play_status();
        if(restart_requested){
            restart_requested=false;
            if(status.level>=0 && status.phase==OPENRAC_PLAY_PLAYING) g->play_travel(status.level);
            else if(g->has_notice()) g->play_notice("Restart is available while playing");
        }
        openrac_pose pose{}; pose.size=sizeof pose;
        if(!g->hero_pose(pose)){ was_alive=true; return; }
        if(qol::new_death(was_alive,pose.health)){
            ++deaths;
            if(show_counter && g->has_notice()) g->play_notice("Deaths this session: "+std::to_string(deaths));
        }
        if(pose.health<=0 && skip_death_scenes && g->has_scenes()){
            int frame=0;
            if(g->scene_playing(&frame)>=0 && frame>=skip_after) g->play_skip();
        }
        was_alive=pose.health>0;
    });
    mod.info("L1+R1+Circle restarts the current planet");
}
OPENRAC_MOD_CPP("death-improvements","0.1.1",load)
