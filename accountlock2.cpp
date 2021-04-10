#include "accountlock2.hpp"


void accountlock2::lock(name target_contract, const time_point_sec &unlock_time, uint8_t days_notice, string public_key_str, string unlocker_str, string locked_auth) {
    require_auth(target_contract);
    if(unlocker_str != "") check(name(unlocker_str) != "accountlock2"_n, "please specify a valid unlocker name"); //this also validates name(unlocker_str)
    check(days_notice >= 0, "days_notice must be positive");
    check(locked_auth == "owner" || locked_auth == "both", "locked_auth must be set to 'owner' or 'both'");
    
    //Time format example: "2020-10-24T16:46:07"
    //time_point_sec unlock_time = current_time_point().sec_since_epoch() + lock_time;
    //time_point_sec unlock_time = lock_time;

		// CHECK PULIC KEY ---------------------------------------------------------------------------------
		check(public_key_str.length() == 53, "Length of public key should be 53");
    
        string pubkey_prefix("EOS");
        auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), public_key_str.begin());
        check(result.first == pubkey_prefix.end(), "Public key should be prefix with EOS");
        auto base58substr = public_key_str.substr(pubkey_prefix.length());
    
        vector<unsigned char> vch;
        check(decode_base58(base58substr, vch), "Decode pubkey failed");
        check(vch.size() == 37, "Invalid public key");
    

    //CREATE THE NEW RECORD
    auto myid = _myLocks.available_primary_key();
    _myLocks.emplace(target_contract, [&](auto& new_row) {
        new_row.id = myid;
        new_row.locked_contract = target_contract;
        new_row.unlock_time = unlock_time;
        new_row.days_notice = days_notice;
        new_row.unlocker = unlocker_str;  
        new_row.locked_auth = locked_auth;  
	    new_row.public_key_str = public_key_str;  
    });

    // change contract permissions
    auto permlev = permission_level{"accountlock2"_n , "eosio.code"_n };

    permission_level_weight perm_weight = {
        .permission = permlev,
        .weight = 1,
    };
    authority myautho= authority{
        .threshold = 1,
        .keys = {},
        .accounts = {perm_weight},
        .waits = {}
    };

    if(locked_auth == "both") {
        action(
                permission_level{ target_contract, "owner"_n },
                "eosio"_n,
                "updateauth"_n,
                make_tuple(target_contract, "active"_n, "owner"_n, myautho)
        ).send();  // updateauth(account, permission, parent, new_auth)
    }		
	if(locked_auth == "owner" || locked_auth == "both") {		
		action(
                permission_level{ target_contract, "owner"_n },
                "eosio"_n,
                "updateauth"_n,
                make_tuple(target_contract, "owner"_n, ""_n, myautho)
        ).send();  // updateauth(account, permission, parent, new_auth)
    }
}


void accountlock2::unlock(name target_contract) {

    for(auto& myindex : _myLocks) {
        if ( myindex.locked_contract == target_contract) {
            if(myindex.unlocker != ""){
                require_auth(name(myindex.unlocker));
            }
            
            //if lock time has expired, activate notice period or restore permission
            check(myindex.unlock_time != eosio::time_point_sec(0) && myindex.unlock_time <  eosio::time_point_sec(current_time_point()), "lock time not expired");
            if (myindex.days_notice > 0) {
                //if days_notice > 0, then activate notice period
                auto lock_itr = _myLocks.find(myindex.id);
                _myLocks.modify(lock_itr, _self, [&](auto &row) {
                    row.unlock_time = time_point_sec(current_time_point() + hours(24 * myindex.days_notice)); //now + days notice
                    row.days_notice = 0;
                });
            } else {
                //restore account permission/s
                string public_key_str = myindex.public_key_str;
                check(public_key_str.length() == 53, "Length of public key should be 53");

                string pubkey_prefix("EOS");
                auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), public_key_str.begin());
                check(result.first == pubkey_prefix.end(), "Public key should be prefix with EOS");
                auto base58substr = public_key_str.substr(pubkey_prefix.length());

                vector<unsigned char> vch;
                check(decode_base58(base58substr, vch), "Decode pubkey failed");
                check(vch.size() == 37, "Invalid public key");

                array<unsigned char,33> pubkey_data;
                copy_n(vch.begin(), 33, pubkey_data.begin());

                checksum160 check_pubkey;
                check_pubkey = ripemd160(reinterpret_cast<char *>(pubkey_data.data()), 33);


                auto permlev = permission_level{ "accountlock2"_n, "eosio.code"_n };
                    
                permission_level_weight perm_weight = {
                        .permission = permlev,
                        .weight = 1,
                };
                    
                signup_public_key pubkey = {
                        .type = 0,
                        .data = pubkey_data,
                };
                key_weight pubkey_weight = {
                        .key = pubkey,
                        .weight = 1,
                };
                authority restoreactive = authority{
                        .threshold = 1,
                        .keys = {pubkey_weight},
                        .accounts = {},
                        .waits = {}
                }; 
                authority restoreowner = authority{
                        .threshold = 1,
                        .keys = {pubkey_weight},
                        .accounts = {perm_weight},
                        .waits = {}
                };

                if(myindex.locked_auth == "both") {	
                    action(
                            permission_level{ target_contract, "owner"_n },
                            "eosio"_n,
                            "updateauth"_n,
                            make_tuple(target_contract, "active"_n, "owner"_n, restoreactive)
                    ).send();
                }		
                if(myindex.locked_auth == "owner" || myindex.locked_auth == "both") {		
                    action(
                            permission_level{ target_contract, "owner"_n },
                            "eosio"_n,
                            "updateauth"_n,
                            make_tuple(target_contract, "owner"_n, ""_n, restoreowner)
                    ).send();  // updateauth(account, permission, parent, new_auth)
                }

                auto lock_itr = _myLocks.find(myindex.id);
                if (lock_itr != _myLocks.end()) {
                    _myLocks.erase(lock_itr);
                }
                
            } 
            require_recipient(target_contract);
            break;
        }
    }
}


void accountlock2::ontransfer(name from, name to, asset quantity, string memo) {
    if(from !=_self && to == _self){ 
          check(to != _self,"please do not send any token to this account");
     } 
}
