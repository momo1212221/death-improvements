#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
openrac::UiPage page;
std::uint16_t previous=0;
bool restart_requested=false, page_requested=false;
bool was_alive=true;
int deaths=0;
}

void load(openrac::Mod& mod) {
    if(mod.game()!="rac1" || !mod.has_play()) throw openrac::Error("Death Improvements requires RAC1 play API");
    g=&mod;

    page=mod.ui_page([](const openrac::UiPage& p,const openrac::UiEvent&){
        openrac::UiContent c;
        c.title="Death Statistics";
        c.items={openrac::UiItem::text("Deaths this session: "+std::to_string(deaths))};
        c.footer={"L1 + R1 + Right opens this page","L1 + R1 + Circle restarts the current planet"};
        p.set(c);
    });

    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto restart=static_cast<std::uint16_t>(OPENRAC_PAD_L1|OPENRAC_PAD_R1|OPENRAC_PAD_CIRCLE);
        const auto stats=static_cast<std::uint16_t>(OPENRAC_PAD_L1|OPENRAC_PAD_R1|OPENRAC_PAD_RIGHT);
        if(qol::combo_pressed(now,previous,restart)) restart_requested=true;
        if(qol::combo_pressed(now,previous,stats)) page_requested=true;
        if((now&restart)==restart) qol::consume_buttons(report,restart);
        if((now&stats)==stats) qol::consume_buttons(report,stats);
        previous=now;
    });

    mod.on_frame([]{
        const auto status=g->play_status();
        if(restart_requested){
            restart_requested=false;
            if(status.level>=0 && status.phase==OPENRAC_PLAY_PLAYING) g->play_travel(status.level);
        }
        if(page_requested){ page_requested=false; page.open(); }

        openrac_pose pose{}; pose.size=sizeof pose;
        if(!g->hero_pose(pose)){ was_alive=true; return; }
        if(qol::new_death(was_alive,pose.health)) ++deaths;
        was_alive=pose.health>0;
    });
}
OPENRAC_MOD_CPP("death-improvements","0.2.0",load)
