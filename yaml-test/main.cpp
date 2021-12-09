#include <bits/stdc++.h>
#include <iostream>
#include <stdexcept>

#include "yaml-cpp/yaml.h"

using namespace std;

class  B_E: public exception
{
    string message;
public:
    explicit B_E(const std::string& msg) : message(msg) {}
    B_E(const B_E&) = default;
    virtual ~B_E(){ }
    virtual  const char* what() const throw ()
    { 
        return message.c_str(); 
    }
};

class B
{
    YAML::Node config;
    string path;
public:
    B(YAML::Node _config)
    {
        config = _config;
    }

    B(const B&)
    {
        cout << "copy constructor" << endl;
    }
    
    B(const string& path) : path(path)
    {
        try
        {
            config = YAML::LoadFile(path);
        }
        catch(YAML::BadFile &e)
        {
            cout<<"read error!"<< e.what() << endl;
            exit(1);
        }
    }

    B get(const string &key)
    {   
        cout << "get1----type:" << key.c_str() <<"  " <<config[key].Type() << " " << YAML::NodeType::Map << endl;
        if(config[key].Type() == YAML::NodeType::Map || config[key].Type() == YAML::NodeType::Sequence)
        {
            return B(config[key]);
        }
        else
        {
            throw B_E("haosdhashdoihoi");
        }
    }

    template <class T>
    T get_at(const int i,const T&default_v)
    {
        cout << "get_at----type:" << config.Type() << " " << config[i].Type() << endl;
        if(config.Type() == YAML::NodeType::Sequence)
        {
            T v = config[i].as<T>();
            return v;
        }
    }

    template <class T>
    int set_at(const int i,const T&v)
    {
        cout << "set_at----type:" << config.Type() << " " << config[i].Type() << endl;
        if(config.Type() == YAML::NodeType::Sequence)
        {
            config[i] = v;
            return 0;
        }

        return -1;
    }

    template <class T>
    T get(const string &key,T const &default_v)
    {
        //读取不存在的node值，报YAML::TypedBadConversion异常
        try
        {
            cout << "get22---type:"<< key.c_str() << "  "<<config[key].Type() << " " << YAML::NodeType::Map << endl;
            if(config[key].Type() == YAML::NodeType::Map)
            {
                throw B_E("haosdhashdoihoi");
            }
            else if(config[key].Type() == YAML::NodeType::Sequence)
            {
                throw B_E("haosdhashdoihoi");
            }
            else if(config[key].Type() == YAML::NodeType::Scalar)
            {
                T v = config[key].as<T>();
                return v;
            }
            else if(config[key].Type() == YAML::NodeType::Undefined)
            {
                return default_v;
            }
        }
        catch(YAML::TypedBadConversion<T> &e)
        {
            return default_v;
        }        
    };

    B set(const string &key)
    {   
        cout << "set1----type:" << key.c_str() <<"  " <<config[key].Type() << " " << YAML::NodeType::Map << endl;
        if(config[key].Type() == YAML::NodeType::Map || config[key].Type() == YAML::NodeType::Sequence)
        {
            return B(config[key]);
        }
        else if(config[key].Type() == YAML::NodeType::Undefined)
        {
            config[key] = 0;
            return B(config[key]);
        }
        else
        {
            throw B_E("haosdhashdoihoi");
        }
    }

    template <class T>
    int set(const string &key,T const &v)
    {
        cout << "set22---type:"<< key.c_str() << "  "<<config[key].Type() << " " << YAML::NodeType::Map << endl;
        if(config[key].Type() == YAML::NodeType::Map)
        {
            throw B_E("Map");
        }
        else if(config[key].Type() == YAML::NodeType::Sequence)
        {
            throw B_E("Sequence");
        }
        else if(config[key].Type() == YAML::NodeType::Scalar || config[key].Type() == YAML::NodeType::Undefined)
        {
            config[key] = v;
        }

        return 0;
    }

    std::size_t size() 
    {
        return config.size();
    }

    int remove(const std::string& key)
    {
        return config.remove(config[key]);
    }

    int save()
    {
        ofstream fout(this->path); //保存config为yaml文件
        fout << config;
        fout.close();
    }

    int save(const std::string &path)
    {
        ofstream fout(path); //保存config为yaml文件
        fout << config;
        fout.close();
    }
};


int main(int argc, char *argv[])
{
    B *cfg = new B("../config1.yaml");

    cout << cfg->size() << endl;

    cout << cfg->get("name",string("wang")) << " " << cfg->get("name111",string("wang111"))  << endl;
    cout << cfg->get("age",1) << "  " << cfg->get("age111",1) << endl;
    cout << cfg->get("arg",1) << "  " << cfg->get("arg",1.0) << endl;

    cfg->set("arg",1123);
    cout << cfg->get("arg",1) << "  " << cfg->get("arg",1.0) << endl;
    cfg->set("skills").set("android",1000) ;
    cout << cfg->get("score").get_at(1,33) << endl;
    cout << cfg->get("skills").get("android",1) << endl;
    cout << cfg->get("skills").get("cpp").get_at(0,1) << endl;
    cout << cfg->get("skills").get("cpp").get_at(2,string("cpp2")) << endl;
    cout << cfg->get("skills").get("tttt").get("abcd").get("c",1) << endl;
    cout << cfg->get("skills").get("tttt").get("abcd").get("d").get_at(2,string("cpp2")) << endl;
    cout << cfg->get("STATE").get("MESSGAE3").get("loop_time_ms",1) << endl;
    
    B state = cfg->get("STATE");
    cout << state.get("MESSGAE3").get("loop_time_ms",1) << endl;

    // B skill = cfg->get("skills");
    // cout << skill.get("cpp").get_at(2,string("cpp2")) << endl;
    // skill.set("android",1123123123);
    cfg->set("skills").set("android",1333);
    cout << cfg->get("skills").get("android",1) << endl;
    
    cfg->set("skills").set("cpp").set_at(0,12333);

    cout << cfg->get("skills").get("cpp").get_at(0,1) << endl;

    cfg->set("skildddls",11);
    cfg->set("skildddls").set("cpp").set_at(0,12333);

    cout << cfg->get("skildddls").get("cpp").get_at(0,1) << endl;

    // try
    // {
    //     cfg->get("skills",1);
    // }
    // catch(const B_E& e)
    // {
    //     std::cerr << "asdasd" << e.what() << '\n';
    // }

    return 0;
}
