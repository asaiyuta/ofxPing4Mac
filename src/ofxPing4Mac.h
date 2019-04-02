//
//  ofxPing4Mac.h
//
//  Created by asai on 2019/03/31 because I was a little bored
//


#ifndef ofxPing4Mac_h
#define ofxPing4Mac_h
#include "ofMain.h"
namespace ofx{
    namespace Ping4Mac{
        namespace thread{
            template<std::size_t num_threads>
            struct pool{
                pool(){
                    for(auto& e : threads) e = std::thread(thread_func);
                }
                ~pool(){
                    is_kill_threads = true;
                    for(auto&& e : threads) e.join();
                }
                
                const std::size_t put(std::function<void()>& func){
                    std::unique_lock<std::mutex> lk(mutex);
                    que.push_back(func);
                    std::size_t s = que.size();
                    return s;
                }
                
                const std::size_t put(std::function<void()>&& func){
                    std::unique_lock<std::mutex> lk(mutex);
                    que.push_back(std::move(func));
                    std::size_t s = que.size();
                    return s;
                }
                
                const bool has_waiting_que()const{
                    return que.size() != 0;
                }
            private:
                std::function<void()> thread_func = std::function<void()>([this](){
                    while(!is_kill_threads){
                        std::function<void()> func;
                        bool is_exist_func;
                        {
                            std::unique_lock<std::mutex> lk(mutex);
                            is_exist_func = !que.empty();
                            if(is_exist_func){
                                func = std::move(que.front());
                                que.pop_front();
                            }
                        }
                        if(is_exist_func) func();
                    }
                });
                
                std::mutex mutex;
                std::atomic_bool is_kill_threads{false};
                std::deque<std::function<void()>> que;
                std::array<std::thread, num_threads> threads;
            };
            
        };
        
        const bool ping(const std::string& ip_addr, const int wait_time = 50){
            std::stringstream cmd("");
            cmd << "ping -c 1 -W " << wait_time << " " << ip_addr << ">/dev/null";
            int res = system(cmd.str().c_str());
            int status = *((char*)(&res) + 1);
            return status == 0;
        }
        
        template<std::size_t NUM_USING_THREADS = 3>
        struct NonBlock{
            NonBlock(){
                ofAddListener(ofEvents().update, this, &NonBlock::update, OF_EVENT_ORDER_BEFORE_APP);
            }
            ~NonBlock(){
                ofRemoveListener(ofEvents().update, this, &NonBlock::update, OF_EVENT_ORDER_BEFORE_APP);
            }
            
            void add(const std::string ip_addr){
                status[ip_addr] = false;
                hosts.push_back(ip_addr);
            }
            void remove(const std::string ip_addr){
                if(status.count(ip_addr) != 0){
                    status.erase(ip_addr);
                    hosts.erase(std::remove(hosts.begin(), hosts.end(), ip_addr), hosts.end());
                }
            }
            
            void setWaitTime(int t){ wait_time = t; }
            
            const bool getState(const std::string& ip)const{
                if(status.count(ip) != 0){
                    return status.at(ip);
                }else{
                    ofLogError("ofxPing4Mac") << ip << " is dont exists!";
                    return false;
                }
            }
            
            const std::string getAllStateStr()const{
                std::stringstream ss("");
                for(const auto& h : hosts){
                    ss << std::setw(15) << h << " : " << status.at(h) << "\n";
                }
                return ss.str();
            }
            
        private:
            void update(ofEventArgs &){
                res_mutex.lock();
                for(const auto& e : result_buffer){
                    if(status.count(e.first) != 0){
                        status.at(e.first) = e.second;
                    }
                }
                result_buffer.clear();
                res_mutex.unlock();
                
                if(hosts.size() != 0){
                    while(!pool.has_waiting_que()){
                        ip_que_mutex.lock();
                        ip_que.push_back(hosts[index]);
                        ip_que_mutex.unlock();
                        
                        pool.put([&](){
                            ip_que_mutex.lock();
                            const std::string ip = ip_que.front();
                            ip_que.pop_front();
                            ip_que_mutex.unlock();
                            const bool s = ping(ip, wait_time);
                            res_mutex.lock();
                            result_buffer.emplace_back(ip, s);
                            res_mutex.unlock();
                        });
                        index = ++index % hosts.size();
                    }
                }
            }
            
            std::atomic_int wait_time{50};
            std::size_t index{0};
            std::mutex res_mutex;
            std::mutex ip_que_mutex;
            std::deque<std::string> ip_que;
            std::vector<std::pair<std::string, bool>> result_buffer;
            std::vector<std::string> hosts;
            std::map<std::string, bool> status;
            thread::pool<NUM_USING_THREADS> pool;
        };
    };
};

template<std::size_t NUM_USE_THREADS>
using ofxPing4MacMulti = ofx::Ping4Mac::NonBlock<NUM_USE_THREADS>;
using ofxPing4Mac = ofx::Ping4Mac::NonBlock<1>;

#endif /* ofxPing4Mac_hs */
