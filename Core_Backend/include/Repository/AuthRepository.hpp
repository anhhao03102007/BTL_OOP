#ifndef AUTH_REPOSITORY_HPP
#define AUTH_REPOSITORY_HPP

#include<string>
#include<optional>
#include<dtos/AuthDTOs.hpp>
#include<model/UserModel.hpp>
#include<database/TursoDB.hpp>


class AuthRepository {
    private:
        TursoDB& db;
    public:
        
        std::optional<UserModel> findByIdentifier(){
            
        } 

        


    


};
#endif