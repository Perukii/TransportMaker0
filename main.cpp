
#include "./cairo_picker/src/cairo_picker.hpp"
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <cmath>

Capr::C_picker picker;
Capr::Function::C_picture base;
struct c_point{ double x = 0,y = 0; };
std::map<std::string, c_point> points;
std::vector<c_point> path;
std::vector<c_point> marks;

const double
    base_w = 2048,
    base_h = 1536,
    dp = 0.3,
    real_w = base_w*dp,
    real_h = base_h*dp
;

const double
    city_mark_size_harf = 5;
;

const double
    cell_wide = 2
;

struct c_point_comp{
    c_point parent;
    c_point point;
    double cost = 0;
};

struct c_color{
    int r;
    int g;
    int b;
};

bool operator<(const c_point& a, const c_point& b){
    return 
        std::floor(a.y)<std::floor(b.y)
        or (std::floor(a.y)==std::floor(b.y) and std::floor(a.x)<std::floor(b.x));
}

bool operator==(const c_point& a, const c_point& b){
    return 
        std::floor(a.y)==std::floor(b.y) and std::floor(a.x)==std::floor(b.x);
}

bool operator<(const c_point_comp& a, const c_point_comp& b){
    return a.point<b.point;
}


void press(uint keyval){
    //if (keyval==GDK_KEY_1) std::cout<<"key pressed"<<std::endl;
}

void release(uint keyval){
    //if (keyval==GDK_KEY_1) std::cout<<"key released"<<std::endl;
}

void motion(uint x, uint y){
    //std::cout<<"motioned"<<std::endl;
}

void bpress(uint button, uint x, uint y){
    if(button == 1) std::cout<<"button pressed["<<x<<":"<<y<<"]"<<std::endl;
    //auto data = base.get_pixel(x/dp, y/dp);
    //std::cout<<get_cost(c_point{0,0}, c_point{double(x),double(y)})<<std::endl;
}

void brelease(uint button, uint x, uint y){
    //if(button == 1) std::cout<<"button released"<<std::endl;
}


c_color get_color(int x, int y){
    auto data = base.get_pixel(x/dp, y/dp);
    return c_color{int(data[0]),int(data[1]),int(data[2])};
    //std::cout<<int(data[0])<<" "<<int(data[1])<<" "<<int(data[2])<<std::endl;
}
c_color get_color(c_point pt){
    return get_color(pt.x, pt.y);
}

double euc(c_point _pfrom, c_point _pto){
    return std::sqrt(std::pow(_pfrom.x-_pto.x,2)+std::pow(_pfrom.y-_pto.y,2));
}

double get_cost(c_point _pfrom, c_point _pto, c_point _parent){
    double dist = std::abs(get_color(_pfrom).g-get_color(_pto).g);
    //double result = (1+dist*0.1)*(std::pow(euc(_pfrom,_parent),0.5));
    double result = (1+dist)*(std::max(std::abs(_pfrom.x-_pto.x), std::abs(_pfrom.y-_pto.y))*std::pow(euc(_pfrom,_parent),0.5));
    return result;
}


void func(Capr::Cairo_cont cr){
    base.set_picture(cr, 0, 0, real_w, real_h);
    cr->paint();
    for(auto it=points.begin();it!=points.end();it++){
        cr->rectangle(it->second.x-city_mark_size_harf, it->second.y-city_mark_size_harf, city_mark_size_harf, city_mark_size_harf);
        cr->set_source_rgb(1,0.5,0.5);
        cr->fill();
    }

    for(int i=0; i<marks.size(); i+=2){
        cr->rectangle(marks[i].x-cell_wide, marks[i].y-cell_wide, cell_wide*2, cell_wide*2);
        cr->set_source_rgb(0.5,0.5,1.0);
        cr->fill();
    }

    cr->set_source_rgb(0,0,0);
    cr->set_line_width(1);

    for(int i=0; i<path.size(); i+=2){
        cr->move_to(path[i].x,path[i].y);
        cr->line_to(path[i+1].x,path[i+1].y);
        cr->stroke();
    }


}

void close_insert(std::set<c_point_comp> * open, std::set<c_point_comp> * close, c_point_comp _pc){
    
    if(close->find(_pc) == close->end()){

        
        open->insert(_pc);
    }
    

}

void path_to(c_point _pst, c_point _ptr){
        
    std::set<c_point_comp> open;
    std::set<c_point_comp> close;
    open.insert(c_point_comp{_pst, _pst, 0});
    auto center_ptr = open.begin();
    c_point center;

    int step =0;

    while((step++)<50000){
        center_ptr = open.begin();
        
        double cost = -1; 
        for(auto it = open.begin(); it != open.end(); it++){
            double cmcost = it->cost;
            if(cost == -1 or cost > cmcost){
                cost = cmcost;
                center_ptr = it;
            }
        }

        center = center_ptr->point;
        
        auto cmp = close.find(*center_ptr);

        if(cmp == close.end())close.insert(*center_ptr);
        else{
            if(cmp->cost > cost){
                close.erase(cmp);
                close.insert(*center_ptr);
            }
        }
        
        //close.insert(*center_ptr);

        open.erase(*center_ptr);

        if(euc(center, _ptr)<=cell_wide){
            break;
        }


        c_point ops[]={
            c_point{center.x,center.y+cell_wide},
            c_point{center.x,center.y-cell_wide},
            c_point{center.x-cell_wide,center.y},
            c_point{center.x+cell_wide,center.y},
            c_point{center.x-cell_wide,center.y-cell_wide},
            c_point{center.x-cell_wide,center.y+cell_wide},
            c_point{center.x+cell_wide,center.y-cell_wide},
            c_point{center.x+cell_wide,center.y+cell_wide}
        };

        for(int i=0;i<8;i++){
            if(ops[i].x < 0 or ops[i].x >=real_w or ops[i].y < 0 or ops[i].y >=real_h or get_color(ops[i]).b > 250) continue;
            c_point_comp cops = c_point_comp{center, ops[i], get_cost(ops[i], _ptr, center)+cost};

            close_insert(&open, &close, cops);
        }
        /*
        c_point
            top = c_point{center.x,center.y+cell_wide},
            btm = c_point{center.x,center.y-cell_wide},
            str = c_point{center.x-cell_wide,center.y},
            end = c_point{center.x+cell_wide,center.y}
        ;
        c_point_comp
            ctop = c_point_comp{center, top, get_cost(top, _ptr)+cost},
            cbtm = c_point_comp{center, btm, get_cost(btm, _ptr)+cost},
            cstr = c_point_comp{center, str, get_cost(str, _ptr)+cost},
            cend = c_point_comp{center, end, get_cost(end, _ptr)+cost}
        ;

        close_insert(&open, &close, ctop);
        close_insert(&open, &close, cbtm);
        close_insert(&open, &close, cstr);
        close_insert(&open, &close, cend);
        */
        

        //std::cout<<center.x<<":"<<center.y<<std::endl;

        

    }

    auto pathmark = close.find({{0,0},center,0});
    if(pathmark == close.end())return;

    while(pathmark->point.x != _pst.x or pathmark->point.y != _pst.y){
        path.push_back(pathmark->point);
        path.push_back(pathmark->parent);
        pathmark = close.find({pathmark->parent,pathmark->parent,0});
    }
/*
    for(auto it:open){
        marks.push_back(it.point);
    }
*/

    std::cout<<open.size()<<std::endl;

}

int main(){
    picker.set_default_size(real_w,real_h);
    picker.set_loop(1);
    picker.set_title("sss");
    picker.set_key_input(press, release);
    picker.set_motion_input(motion);
    picker.set_button_input(bpress, brelease);

    base.open_file("./base.png");

    points.emplace("city5",c_point{172*3.3*dp,249*3.3*dp});
    points.emplace("city0",c_point{362*3.3*dp,212*3.3*dp});
    points.emplace("city1",c_point{283*3.3*dp,429*3.3*dp});
    points.emplace("city2",c_point{542*3.3*dp,323*3.3*dp});
    points.emplace("city3",c_point{418*3.3*dp,283*3.3*dp});
    points.emplace("city4",c_point{516*3.3*dp,145*3.3*dp});

    path_to(points["city3"],points["city0"]);
    path_to(points["city3"],points["city1"]);
    path_to(points["city3"],points["city2"]);
    path_to(points["city3"],points["city4"]);
    path_to(points["city2"],points["city4"]);
    path_to(points["city0"],points["city5"]);
    //path_to(points["city1"],points["city4"]);
    //path.push_back(c_point{1,1});
    //path.push_back(c_point{100,100});
    picker.run(func);
}