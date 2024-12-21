#include <bits/stdc++.h>
using namespace std;

using FLOAT_t = float;
using DOUBLE_t = double;

template<int N, int K>
struct Fixed {
    static_assert(N > K, "N should be greater than K");
    static_assert(N <= 64, "Too large N");

    using underlying_type = std::conditional_t<(N<=8), int8_t,
                           std::conditional_t<(N<=16), int16_t,
                           std::conditional_t<(N<=32), int32_t,int64_t>>>;

    underlying_type v;

    constexpr Fixed(): v(0) {}
    constexpr Fixed(int iv) : v((underlying_type)(iv << K)) {}
    constexpr Fixed(float fv) : v((underlying_type)(fv * (1 << K))) {}
    constexpr Fixed(double dv) : v((underlying_type)(dv * (1 << K))) {}

    static constexpr Fixed from_raw(underlying_type x) {
        Fixed ret;
        ret.v = x;
        return ret;
    }

    explicit operator double() const {
        return (double)v / (1 << K);
    }

    bool operator==(const Fixed &other) const {return v==other.v;}
    bool operator!=(const Fixed &other) const {return v!=other.v;}
    bool operator<(const Fixed &other) const {return v<other.v;}
    bool operator>(const Fixed &other) const {return v>other.v;}
    bool operator<=(const Fixed &other) const {return v<=other.v;}
    bool operator>=(const Fixed &other) const {return v>=other.v;}

    friend Fixed operator+(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v+b.v);
    }
    friend Fixed operator-(Fixed a, Fixed b) {
        return Fixed::from_raw(a.v-b.v);
    }
    friend Fixed operator-(Fixed x) {
        return Fixed::from_raw(-x.v);
    }
    friend Fixed operator*(Fixed a, Fixed b) {
        __int128 temp=(__int128)a.v*b.v;
        return Fixed::from_raw((underlying_type)(temp>>K));
    }
    friend Fixed operator/(Fixed a, Fixed b) {
        __int128 temp=((__int128)a.v<<K)/b.v;
        return Fixed::from_raw((underlying_type)temp);
    }

    Fixed &operator+=(Fixed b) { *this = *this+b; return *this; }
    Fixed &operator-=(Fixed b) { *this = *this-b; return *this; }
    Fixed &operator*=(Fixed b) { *this = *this*b; return *this; }
    Fixed &operator/=(Fixed b) { *this = *this/b; return *this; }

    friend std::ostream &operator<<(std::ostream &out, Fixed x) {
        double val=(double)x;
        return out<<val;
    }
};

template<int N,int K>
Fixed<N,K> abs(Fixed<N,K> x) {
    if (x.v<0) x.v=-x.v;
    return x;
}

template<int N, int K>
struct FastFixed {
    using underlying_type = std::conditional_t<(N<=8), int_fast8_t,
                           std::conditional_t<(N<=16), int_fast16_t,
                           std::conditional_t<(N<=32), int_fast32_t,int_fast64_t>>>;

    static_assert(sizeof(underlying_type)*8 >= (size_t)N, "No suitable fast type");

    underlying_type v;

    constexpr FastFixed(): v(0) {}
    constexpr FastFixed(int iv): v((underlying_type)(iv<<K)) {}
    constexpr FastFixed(float fv): v((underlying_type)(fv*(1<<K))) {}
    constexpr FastFixed(double dv): v((underlying_type)(dv*(1<<K))) {}

    static constexpr FastFixed from_raw(underlying_type x) {
        FastFixed ret;
        ret.v=x;
        return ret;
    }

    explicit operator double() const {
        return (double)v / (1 << K);
    }

    bool operator==(const FastFixed &o) const {return v==o.v;}
    bool operator!=(const FastFixed &o) const {return v!=o.v;}
    bool operator<(const FastFixed &o) const {return v<o.v;}
    bool operator>(const FastFixed &o) const {return v>o.v;}
    bool operator<=(const FastFixed &o) const {return v<=o.v;}
    bool operator>=(const FastFixed &o) const {return v>=o.v;}

    friend FastFixed operator+(FastFixed a, FastFixed b) {
        return FastFixed::from_raw(a.v+b.v);
    }
    friend FastFixed operator-(FastFixed a, FastFixed b) {
        return FastFixed::from_raw(a.v-b.v);
    }
    friend FastFixed operator-(FastFixed x) {
        return FastFixed::from_raw(-x.v);
    }
    friend FastFixed operator*(FastFixed a, FastFixed b) {
        __int128 temp=(__int128)a.v*b.v;
        return FastFixed::from_raw((typename FastFixed::underlying_type)(temp>>K));
    }
    friend FastFixed operator/(FastFixed a, FastFixed b) {
        __int128 temp=((__int128)a.v<<K)/b.v;
        return FastFixed::from_raw((typename FastFixed::underlying_type)temp);
    }

    FastFixed &operator+=(FastFixed b){*this=*this+b;return *this;}
    FastFixed &operator-=(FastFixed b){*this=*this-b;return *this;}
    FastFixed &operator*=(FastFixed b){*this=*this*b;return *this;}
    FastFixed &operator/=(FastFixed b){*this=*this/b;return *this;}

    friend std::ostream &operator<<(std::ostream &out, FastFixed x) {
        double val=(double)x;
        return out<<val;
    }
};

template<int N,int K>
FastFixed<N,K> abs(FastFixed<N,K> x) {
    if(x.v<0)x.v=-x.v;
    return x;
}

static constexpr array<pair<int,int>,4> deltas{{{-1,0},{1,0},{0,-1},{0,1}}};

struct BaseSimulatorInterface {
    virtual void load_input(const string &filename)=0;
    virtual void run_simulation(size_t T)=0;
    virtual ~BaseSimulatorInterface(){}
};

template<typename PType, typename VType, typename VFlowType, size_t N, size_t M>
class FluidSimulator {
public:
    FluidSimulator() {
        g=(PType)0.1;
        viscosity=(PType)0.0;
        for (auto &r:rho) r=(PType)0;
        for(size_t x=0;x<N;x++){
            for(size_t y=0;y<M+1;y++){
                field[x][y]='#';
            }
        }
    }

    void load_input(const string &filename) {
        ifstream in(filename);
        if(!in){
            cerr<<"Error: can't open file "<<filename<<"\n";
            exit(1);
        }

        size_t inputN, inputM;
        double gd, visc;
        in >> inputN >> inputM;
        in >> gd;
        in >> visc;
        int ccount;
        in >> ccount;
        for(int i=0;i<ccount;i++){
            char c;double d; in>>c>>d;
            rho[(unsigned char)c]=(PType)d;
        }

        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        for(size_t x=0;x<N;x++){
            for(size_t y=0;y<M;y++){
                char c; in.get(c);
                field[x][y]=c;
            }
            field[x][M]='\0';
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        g=(PType)gd;
        viscosity=(PType)visc;

        for(size_t x=0;x<N;x++){
            for(size_t y=0;y<M;y++){
                dirs[x][y]=0;
                if(field[x][y]=='#') continue;
                for (auto [dx,dy]:deltas){
                    int nx=(int)x+dx, ny=(int)y+dy;
                    if(nx>=0&&(size_t)nx<N && ny>=0 && (size_t)ny<M && field[nx][ny]!='#')
                        dirs[x][y]++;
                }
            }
        }
    }

    void run_simulation(size_t T) {
        // ios::sync_with_stdio(false);
        cin.tie(nullptr);

        mt19937 rnd(1337);
        auto random01 = [&]() {
            double val = (double)(rnd() & 0xFFFF) / 65536.0;
            return (PType)val;
        };

        auto min_ = [&](auto a, auto b) {
            return (a < b) ? a : b;
        };

        auto apply_viscosity = [&](auto &vf) {
            PType factor = (PType)(1.0 - (double)viscosity);
            for (size_t x = 0; x < N; x++) {
                for (size_t y = 0; y < M; y++) {
                    for (size_t i = 0; i < 4; i++) {
                        vf.v[x][y][i] *= factor;
                    }
                }
            }
        };

        static int last_use[N][M];
        auto propagate_flow = [&](auto self, int x, int y, PType lim, auto &last_use) -> tuple<PType,bool,pair<int,int>> {
            last_use[x][y] = UT - 1;
            PType ret(0);
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || nx >= (int)N || ny < 0 || ny >= (int)M) continue;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                    auto cap = velocity.get(x, y, dx, dy);
                    auto flow = velocity_flow.get(x, y, dx, dy);
                    if (flow == cap) continue;
                    auto vp = min_(lim, cap - flow);
                    if (last_use[nx][ny] == UT - 1) {
                        velocity_flow.add(x, y, dx, dy, vp);
                        last_use[x][y] = UT;
                        return make_tuple(vp, true, pair<int,int>(nx, ny));
                    }
                    auto [t, prop, end] = self(self, nx, ny, vp, last_use);
                    ret += t;
                    if (prop) {
                        velocity_flow.add(x, y, dx, dy, t);
                        last_use[x][y] = UT;
                        return make_tuple(t, prop && end != pair<int,int>(x, y), end);
                    }
                }
            }
            last_use[x][y] = UT;
            return make_tuple(ret,false,pair<int,int>(0,0));
        };

        auto propagate_stop = [&](auto self,int x,int y,bool force,auto &last_use){
            if(!force) {
                bool stop = true;
                for(auto[dx,dy]:deltas){
                    int nx=x+dx, ny=y+dy;
                    if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                    if(field[nx][ny]!='#' && last_use[nx][ny]<UT-1 && velocity.get(x,y,dx,dy)>PType(0)) {stop=false;break;}
                }
                if(!stop) return;
            }
            last_use[x][y]=UT;
            for(auto[dx,dy]:deltas){
                int nx=x+dx, ny=y+dy;
                if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                if(field[nx][ny]=='#'||last_use[nx][ny]==UT||velocity.get(x,y,dx,dy)>PType(0)) continue;
                self(self,nx,ny,false,last_use);
            }
        };

        auto move_prob = [&](int x,int y,auto &last_use) {
            PType sum(0);
            for (size_t i=0; i<deltas.size(); i++) {
                auto[dx,dy] = deltas[i];
                int nx=x+dx,ny=y+dy;
                if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                if(field[nx][ny]=='#' || last_use[nx][ny]==UT) continue;
                auto v=velocity.get(x,y,dx,dy);
                if(v<PType(0)) continue;
                sum+=v;
            }
            return sum;
        };

        struct ParticleParams {
            char type;
            PType cur_p;
            array<VFlowType,4> v;
            void swap_with(FluidSimulator &sim,int x,int y) {
                std::swap(sim.field[x][y], type);
                std::swap(sim.p[x][y], cur_p);
                for (size_t i=0;i<4;i++){
                    std::swap(sim.velocity.v[x][y][i], v[i]);
                }
            }
        };

        auto propagate_move = [&](auto self,int x,int y,bool is_first,auto &last_use)->bool {
            last_use[x][y]=UT-(is_first?1:0);
            bool ret=false;
            int nx=-1,ny=-1;
            do {
                std::array<PType,4> tres;
                PType sum(0);
                for(size_t i=0;i<deltas.size();i++){
                    auto[dx,dy]=deltas[i];
                    int nx2=x+dx,ny2=y+dy;
                    if(nx2<0||nx2>=(int)N||ny2<0||ny2>=(int)M){tres[i]=sum;continue;}
                    if(field[nx2][ny2]=='#'||last_use[nx2][ny2]==UT){tres[i]=sum;continue;}
                    auto v=velocity.get(x,y,dx,dy);
                    if(v<PType(0)){tres[i]=sum;continue;}
                    sum+=v;
                    tres[i]=sum;
                }

                if(sum==PType(0)) break;

                PType p = random01()*sum;
                size_t d=0;
                for(;d<4;d++){
                    if(tres[d]>p) break;
                }

                auto[dx,dy]=deltas[d];
                nx=x+dx;ny=y+dy;
                ret=(last_use[nx][ny]==UT-1||self(self,nx,ny,false,last_use));
            } while(!ret);
            last_use[x][y]=UT;
            for(auto[dx,dy]:deltas){
                int nx2=x+dx,ny2=y+dy;
                if(nx2<0||nx2>=(int)N||ny2<0||ny2>=(int)M) continue;
                if(field[nx2][ny2]!='#' && last_use[nx2][ny2]<UT-1 && velocity.get(x,y,dx,dy)<PType(0)){
                    propagate_stop(propagate_stop,nx2,ny2,false,last_use);
                }
            }
            if(ret && !is_first) {
                ParticleParams pp{};
                pp.type=field[x][y];
                pp.cur_p=p[x][y];
                for(size_t i=0;i<4;i++){
                    pp.v[i]=velocity.v[x][y][i];
                }

                pp.swap_with(*this,nx,ny);
                pp.swap_with(*this,x,y);
            }
            return ret;
        };

        if(rho[(unsigned char)' ']==(PType)0) rho[(unsigned char)' ']=(PType)0.01;
        if(rho[(unsigned char)'.']==(PType)0) rho[(unsigned char)'.']=(PType)1000;

        for(size_t i=0;i<T;i++){
            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    if(x+1<N && field[x+1][y]!='#'){
                        velocity.add((int)x,(int)y,1,0,g);
                    }
                }
            }

            memcpy(old_p,p,sizeof(p));

            PType total_delta_p(0);

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    for(auto[dx,dy]:deltas){
                        int nx=x+dx, ny=y+dy;
                        if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                        if(field[nx][ny]!='#' && old_p[nx][ny]<old_p[x][y]) {
                            auto delta_p = old_p[x][y]-old_p[nx][ny];
                            auto force = delta_p;
                            auto &contr = velocity.get(nx,ny,-dx,-dy);
                            if(contr*rho[(unsigned char)field[nx][ny]] >= force) {
                                contr -= force/rho[(unsigned char)field[nx][ny]];
                                continue;
                            }
                            force -= contr*rho[(unsigned char)field[nx][ny]];
                            contr = (PType)0;
                            velocity.add(x,y,dx,dy, force/rho[(unsigned char)field[x][y]]);
                            p[x][y]-=force/(PType)dirs[x][y];
                            total_delta_p-=force/(PType)dirs[x][y];
                        }
                    }
                }
            }

            velocity_flow={};
            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    last_use[x][y]=-9999999;
                }
            }

            bool prop=false;
            do {
                UT+=2;
                prop=false;
                for(size_t x=0;x<N;x++){
                    for(size_t y=0;y<M;y++){
                        if(field[x][y]!='#' && last_use[x][y]!=UT) {
                            auto[t,local_prop,_]=propagate_flow(propagate_flow,x,y,(PType)1,last_use);
                            if(t>0) prop=true;
                        }
                    }
                }
            } while(prop);

            apply_viscosity(velocity);
            apply_viscosity(velocity_flow);

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    for(auto[dx,dy]:deltas){
                        auto old_v = velocity.get(x,y,dx,dy);
                        auto new_v = velocity_flow.get(x,y,dx,dy);
                        if(old_v>PType(0)) {
                            velocity.get(x,y,dx,dy)=new_v;
                            auto force=(old_v - new_v)*rho[(unsigned char)field[x][y]];
                            if(field[x][y]=='.') force*= (PType)0.8;
                            int nx=x+dx, ny=y+dy;
                            if(nx<0||nx>=(int)N||ny<0||ny>=(int)M||field[nx][ny]=='#') {
                                p[x][y]+=force/(PType)dirs[x][y];
                                total_delta_p+=force/(PType)dirs[x][y];
                            } else {
                                p[nx][ny]+=force/(PType)dirs[nx][ny];
                                total_delta_p+=force/(PType)dirs[nx][ny];
                            }
                        }
                    }
                }
            }

            UT+=2;
            bool changed=false;
            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]!='#' && last_use[x][y]!=UT) {
                        if(random01()<move_prob(x,y,last_use)) {
                            if(propagate_move(propagate_move,x,y,true,last_use)) changed=true;
                        } else {
                            propagate_stop(propagate_stop,x,y,true,last_use);
                        }
                    }
                }
            }

            if(changed) {
                // std::cout << "\x1b[2J\x1b[H";
                std::cout << "Tick " << i << ":\n";
                for(size_t x=0;x<N;x++) {
                    cout << field[x] << "\n";
                }
            }
        }
    }


private:
    char field[N][M+1];
    PType p[N][M]{}, old_p[N][M]{};
    PType rho[256]{};
    int dirs[N][M]{};
    int UT=0;
    PType g;
    PType viscosity;

    struct VectorField {
        array<VFlowType,4> v[N][M]{};
        VFlowType &add(int x,int y,int dx,int dy,VFlowType dv) {
            return get(x,y,dx,dy)+=dv;
        }
        VFlowType &get(int x,int y,int dx,int dy) {
            for(size_t i=0;i<4;i++){
                if(deltas[i].first==dx && deltas[i].second==dy) return v[x][y][i];
            }
            return v[x][y][0];
        }
    } velocity, velocity_flow;
};

template<typename PType, typename VType, typename VFlowType>
class FluidSimulatorDynamic {
public:
    FluidSimulatorDynamic(size_t N_, size_t M_):N(N_),M(M_){
        field.resize(N, std::string(M,'#'));
        p.resize(N,vector<PType>(M,(PType)0));
        old_p.resize(N,vector<PType>(M,(PType)0));
        velocity.v.resize(N, vector<array<VFlowType,4>>(M));
        velocity_flow.v.resize(N, vector<array<VFlowType,4>>(M));
        dirs.resize(N,vector<int>(M,0));
        g=(PType)0.1;
        viscosity=(PType)0.0;
        for(auto &r:rho) r=(PType)0;
    }

    void load_input(const string &filename) {
        ifstream in(filename);
        if(!in){
            cerr<<"Error: cannot open "<<filename<<"\n";
            exit(1);
        }
        size_t inputN,inputM;
        double gd,visc;
        in>>inputN>>inputM;
        in>>gd;
        in>>visc;
        int ccount; in>>ccount;
        for(int i=0;i<ccount;i++){
            char c; double d; in>>c>>d;
            rho[(unsigned char)c]=(PType)d;
        }
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        for(size_t x=0;x<N;x++){
            for(size_t y=0;y<M;y++){
                char c; in.get(c);
                field[x][y]=c;
            }
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        g=(PType)gd;
        viscosity=(PType)visc;
        for(size_t x=0;x<N;x++){
            for(size_t y=0;y<M;y++){
                if(field[x][y]=='#') continue;
                for(auto [dx,dy]:deltas){
                    int nx=(int)x+dx, ny=(int)y+dy;
                    if(nx>=0&&(size_t)nx<N && ny>=0&&(size_t)ny<M && field[nx][ny]!='#')
                        dirs[x][y]++;
                }
            }
        }
    }

    void run_simulation(size_t T) {
        // ios::sync_with_stdio(false);
        cin.tie(nullptr);

        mt19937 rnd(1337);
        auto random01 = [&]() {
            double val = (double)(rnd() & 0xFFFF)/65536.0;
            return (PType)val;
        };

        auto min_ = [&](auto a, auto b) {
            return (a < b) ? a : b;
        };

        auto apply_viscosity = [&](auto &vf) {
            PType factor = (PType)(1.0 - (double)viscosity);
            for (size_t x = 0; x < N; x++) {
                for (size_t y = 0; y < M; y++) {
                    for (size_t i = 0; i < 4; i++) {
                        vf.v[x][y][i] *= factor;
                    }
                }
            }
        };

        vector<vector<int>> last_use(N, vector<int>(M, -9999999));

        auto propagate_flow = [&](auto self,int x,int y,PType lim,auto &last_use)->tuple<PType,bool,pair<int,int>> {
            last_use[x][y]=UT-1;
            PType ret(0);
            for(auto[dx,dy]:deltas){
                int nx=x+dx, ny=y+dy;
                if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                if(field[nx][ny]!='#' && last_use[nx][ny]<UT) {
                    auto cap=velocity.get(x,y,dx,dy);
                    auto flow=velocity_flow.get(x,y,dx,dy);
                    if(flow==cap) continue;
                    auto vp=min_(lim,cap-flow);
                    if(last_use[nx][ny]==UT-1) {
                        velocity_flow.add(x,y,dx,dy,vp);
                        last_use[x][y]=UT;
                        return make_tuple(vp,true,pair<int,int>(nx,ny));
                    }
                    auto[t,prop,end]=self(self,nx,ny,vp,last_use);
                    ret+=t;
                    if(prop) {
                        velocity_flow.add(x,y,dx,dy,t);
                        last_use[x][y]=UT;
                        return make_tuple(t, prop && end!=pair<int,int>(x,y), end);
                    }
                }
            }
            last_use[x][y]=UT;
            return make_tuple(ret,false,pair<int,int>(0,0));
        };

        auto propagate_stop=[&](auto self,int x,int y,bool force,auto &last_use) {
            if(!force) {
                bool stop=true;
                for(auto[dx,dy]:deltas) {
                    int nx=x+dx, ny=y+dy;
                    if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                    if(field[nx][ny]!='#'&&last_use[nx][ny]<UT-1 && velocity.get(x,y,dx,dy)>PType(0)){stop=false;break;}
                }
                if(!stop)return;
            }
            last_use[x][y]=UT;
            for(auto[dx,dy]:deltas){
                int nx=x+dx, ny=y+dy;
                if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                if(field[nx][ny]=='#'||last_use[nx][ny]==UT||velocity.get(x,y,dx,dy)>PType(0)) continue;
                self(self,nx,ny,false,last_use);
            }
        };

        auto move_prob=[&](int x,int y,auto &last_use){
            PType sum(0);
            for(size_t i=0;i<deltas.size();i++){
                auto[dx,dy]=deltas[i];
                int nx=x+dx,ny=y+dy;
                if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                if(field[nx][ny]=='#'||last_use[nx][ny]==UT) continue;
                auto v=velocity.get(x,y,dx,dy);
                if(v<PType(0)) continue;
                sum+=v;
            }
            return sum;
        };

        struct ParticleParams {
            char type;
            PType cur_p;
            array<VFlowType,4> v;
            void swap_with(FluidSimulatorDynamic &sim,int x,int y) {
                std::swap(sim.field[x][y], type);
                std::swap(sim.p[x][y], cur_p);
                for(size_t i=0;i<4;i++){
                    std::swap(sim.velocity.v[x][y][i], v[i]);
                }
            }
        };

        auto propagate_move=[&](auto self,int x,int y,bool is_first,auto &last_use)->bool {
            last_use[x][y]=UT-(is_first?1:0);
            bool ret=false;
            int nx=-1, ny=-1;
            do {
                std::array<PType,4> tres;
                PType sum(0);
                for(size_t i=0;i<4;i++){
                    auto[dx,dy]=deltas[i];
                    int nx2=x+dx,ny2=y+dy;
                    if(nx2<0||nx2>=(int)N||ny2<0||ny2>=(int)M){tres[i]=sum;continue;}
                    if(field[nx2][ny2]=='#'||last_use[nx2][ny2]==UT){tres[i]=sum;continue;}
                    auto v=velocity.get(x,y,dx,dy);
                    if(v<PType(0)){tres[i]=sum;continue;}
                    sum+=v;
                    tres[i]=sum;
                }

                if(sum==PType(0)) break;

                PType p=random01()*sum;
                size_t d=0;
                for(;d<4;d++){
                    if(tres[d]>p)break;
                }

                auto[dx,dy]=deltas[d];
                nx=x+dx; ny=y+dy;
                ret=(last_use[nx][ny]==UT-1||self(self,nx,ny,false,last_use));
            }while(!ret);
            last_use[x][y]=UT;
            for(auto[dx,dy]:deltas){
                int nx2=x+dx, ny2=y+dy;
                if(nx2<0||nx2>=(int)N||ny2<0||ny2>=(int)M) continue;
                if(field[nx2][ny2]!='#' && last_use[nx2][ny2]<UT-1 && velocity.get(x,y,dx,dy)<PType(0)){
                    propagate_stop(propagate_stop,nx2,ny2,false,last_use);
                }
            }
            if(ret && !is_first) {
                ParticleParams pp{};
                pp.type=field[x][y];
                pp.cur_p=p[x][y];
                for(size_t i=0;i<4;i++){
                    pp.v[i]=velocity.v[x][y][i];
                }
                pp.swap_with(*this,nx,ny);
                pp.swap_with(*this,x,y);
            }
            return ret;
        };

        if(rho[(unsigned char)' ']==(PType)0) rho[(unsigned char)' ']=(PType)0.01;
        if(rho[(unsigned char)'.']==(PType)0) rho[(unsigned char)'.']=(PType)1000;

        for(size_t i=0;i<T;i++){
            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    if(x+1<N && field[x+1][y]!='#'){
                        velocity.add((int)x,(int)y,1,0,g);
                    }
                }
            }

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    old_p[x][y]=p[x][y];
                }
            }

            PType total_delta_p(0);

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    for(auto[dx,dy]:deltas){
                        int nx=x+dx, ny=y+dy;
                        if(nx<0||nx>=(int)N||ny<0||ny>=(int)M) continue;
                        if(field[nx][ny]!='#' && old_p[nx][ny]<old_p[x][y]) {
                            auto delta_p=old_p[x][y]-old_p[nx][ny];
                            auto force=delta_p;
                            auto &contr=velocity.get(nx,ny,-dx,-dy);
                            if(contr*rho[(unsigned char)field[nx][ny]]>=force){
                                contr-=force/rho[(unsigned char)field[nx][ny]];
                                continue;
                            }
                            force-=contr*rho[(unsigned char)field[nx][ny]];
                            contr=(PType)0;
                            velocity.add(x,y,dx,dy, force/rho[(unsigned char)field[x][y]]);
                            p[x][y]-=force/(PType)dirs[x][y];
                            total_delta_p-=force/(PType)dirs[x][y];
                        }
                    }
                }
            }

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    for(size_t i=0;i<4;i++){
                        velocity_flow.v[x][y][i]=(VFlowType)0;
                    }
                }
            }

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    last_use[x][y]=-9999999;
                }
            }

            bool prop=false;
            do {
                UT+=2;
                prop=false;
                for(size_t x=0;x<N;x++){
                    for(size_t y=0;y<M;y++){
                        if(field[x][y]!='#' && last_use[x][y]!=UT) {
                            auto[t,local_prop,_]=propagate_flow(propagate_flow,x,y,(PType)1,last_use);
                            if(t>0) prop=true;
                        }
                    }
                }
            } while(prop);

            apply_viscosity(velocity);
            apply_viscosity(velocity_flow);

            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]=='#') continue;
                    for(auto[dx,dy]:deltas){
                        auto old_v=velocity.get(x,y,dx,dy);
                        auto new_v=velocity_flow.get(x,y,dx,dy);
                        if(old_v>PType(0)) {
                            velocity.get(x,y,dx,dy)=new_v;
                            auto force=(old_v - new_v)*rho[(unsigned char)field[x][y]];
                            if(field[x][y]=='.') force*=(PType)0.8;
                            int nx=x+dx, ny=y+dy;
                            if(nx<0||nx>=(int)N||ny<0||ny>=(int)M||field[nx][ny]=='#'){
                                p[x][y]+=force/(PType)dirs[x][y];
                                total_delta_p+=force/(PType)dirs[x][y];
                            } else {
                                p[nx][ny]+=force/(PType)dirs[nx][ny];
                                total_delta_p+=force/(PType)dirs[nx][ny];
                            }
                        }
                    }
                }
            }

            UT+=2;
            bool changed=false;
            for(size_t x=0;x<N;x++){
                for(size_t y=0;y<M;y++){
                    if(field[x][y]!='#' && last_use[x][y]!=UT) {
                        if(random01()<move_prob(x,y,last_use)) {
                            if(propagate_move(propagate_move,x,y,true,last_use)) changed=true;
                        } else {
                            propagate_stop(propagate_stop,x,y,true,last_use);
                        }
                    }
                }
            }

            if(changed) {
                // std::cout << "\x1b[2J\x1b[H";
                cout << "Tick " << i << ":\n";
                for(size_t x=0;x<N;x++){
                    for(size_t y=0;y<M;y++) cout << field[x][y];
                    cout << "\n";
                }
            }
        }
    }

private:
    size_t N,M;
    vector<string> field;
    vector<vector<PType>> p, old_p;
    struct VectorField {
        vector<vector<array<VFlowType,4>>> v;
        VFlowType &add(int x,int y,int dx,int dy,VFlowType dv) {
            return get(x,y,dx,dy)+=dv;
        }
        VFlowType &get(int x,int y,int dx,int dy) {
            for(size_t i=0;i<4;i++){
                if(deltas[i].first==dx && deltas[i].second==dy) return v[x][y][i];
            }
            return v[x][y][0];
        }
    } velocity, velocity_flow;
    PType rho[256];
    vector<vector<int>> dirs;
    int UT=0;
    PType g;
    PType viscosity;
};

struct SimulatorImplBase : BaseSimulatorInterface {
    virtual ~SimulatorImplBase(){}
};

template<typename PType, typename VType, typename VFlowType, size_t N, size_t M>
struct SimulatorImpl : SimulatorImplBase {
    FluidSimulator<PType,VType,VFlowType,N,M> sim;
    void load_input(const string &filename) override {
        sim.load_input(filename);
    }
    void run_simulation(size_t T) override {
        sim.run_simulation(T);
    }
};

template<typename PType, typename VType, typename VFlowType>
struct SimulatorImplDynamic : SimulatorImplBase {
    unique_ptr<FluidSimulatorDynamic<PType,VType,VFlowType>> sim;
    SimulatorImplDynamic(size_t n,size_t m) {
        sim=make_unique<FluidSimulatorDynamic<PType,VType,VFlowType>>(n,m);
    }
    void load_input(const string &filename) override {
        sim->load_input(filename);
    }
    void run_simulation(size_t T) override {
        sim->run_simulation(T);
    }
};

static unique_ptr<BaseSimulatorInterface> create_simulator(
    const string &p_type_str,
    const string &v_type_str,
    const string &v_flow_type_str,
    size_t inputN, size_t inputM)
{
    if((inputN==36 && inputM==84)) {
        if(p_type_str=="FLOAT" && v_type_str=="FLOAT" && v_flow_type_str=="FLOAT")
            return make_unique<SimulatorImpl<FLOAT_t,FLOAT_t,FLOAT_t,36,84>>();
        if(p_type_str=="DOUBLE" && v_type_str=="DOUBLE" && v_flow_type_str=="DOUBLE")
            return make_unique<SimulatorImpl<DOUBLE_t,DOUBLE_t,DOUBLE_t,36,84>>();
        if(p_type_str=="FIXED(32,16)"&&v_type_str=="FIXED(32,16)"&&v_flow_type_str=="FIXED(32,16)")
            return make_unique<SimulatorImpl<Fixed<32,16>,Fixed<32,16>,Fixed<32,16>,36,84>>();
        if(p_type_str=="FAST_FIXED(32,16)"&&v_type_str=="FAST_FIXED(32,16)"&&v_flow_type_str=="FAST_FIXED(32,16)")
            return make_unique<SimulatorImpl<FastFixed<32,16>,FastFixed<32,16>,FastFixed<32,16>,36,84>>();

        throw runtime_error("No matching types for S(36,84)");
    } else if((inputN==14 && inputM==5)){
        if(p_type_str=="FLOAT" && v_type_str=="FLOAT" && v_flow_type_str=="FLOAT")
            return make_unique<SimulatorImpl<FLOAT_t,FLOAT_t,FLOAT_t,14,5>>();
        throw runtime_error("No matching types for S(14,5)");
    } else {
        if(p_type_str=="FLOAT" && v_type_str=="FLOAT" && v_flow_type_str=="FLOAT")
            return make_unique<SimulatorImplDynamic<FLOAT_t,FLOAT_t,FLOAT_t>>(inputN,inputM);
        if(p_type_str=="DOUBLE" && v_type_str=="DOUBLE" && v_flow_type_str=="DOUBLE")
            return make_unique<SimulatorImplDynamic<DOUBLE_t,DOUBLE_t,DOUBLE_t>>(inputN,inputM);
        if(p_type_str=="FIXED(32,16)"&&v_type_str=="FIXED(32,16)"&&v_flow_type_str=="FIXED(32,16)")
            return make_unique<SimulatorImplDynamic<Fixed<32,16>,Fixed<32,16>,Fixed<32,16>>>(inputN,inputM);
        if(p_type_str=="FAST_FIXED(32,16)"&&v_type_str=="FAST_FIXED(32,16)"&&v_flow_type_str=="FAST_FIXED(32,16)")
            return make_unique<SimulatorImplDynamic<FastFixed<32,16>,FastFixed<32,16>,FastFixed<32,16>>>(inputN,inputM);

        throw runtime_error("No matching types for dynamic size");
    }
}

int main(int argc,char** argv){
    // ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string p_type_str="FLOAT";
    string v_type_str="FLOAT";
    string v_flow_type_str="FLOAT";
    string filename="input.txt";

    for(int i=1;i<argc;i++){
        string arg=argv[i];
        if(arg.rfind("--p-type=",0)==0)p_type_str=arg.substr(9);
        else if(arg.rfind("--v-type=",0)==0)v_type_str=arg.substr(9);
        else if(arg.rfind("--v-flow-type=",0)==0)v_flow_type_str=arg.substr(14);
        else filename=arg;
    }

    ifstream in(filename);
    if(!in){
        cerr<<"Error: cannot open "<<filename<<"\n";
        return 1;
    }
    size_t inputN,inputM;
    in>>inputN>>inputM;
    in.close();

    auto sim=create_simulator(p_type_str,v_type_str,v_flow_type_str,inputN,inputM);
    sim->load_input(filename);
    sim->run_simulation(1500);

    return 0;
}
