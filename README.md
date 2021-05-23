# ACCOUNT-LOCK V.2
Temporary immutablity for EOSIO smart contracts.
Locks your EOSIO account and ensures that your smart contract is immutable for a given time period

- A secure contract that can only restore the owner and active keys after a pre-defined time.

- Deployed as a smart contract on the Eos, UX Network and Telos mainnets

- Ensures that no-one has control of a given account while locked.

- Specify a notice period between the time the unlocking process is started and the time it will actually be possible to unlock the contract.




## GETTING LOCKED

Follow these steps to provide temporary immutability to your account:

1. Add the **accountlock2@eosio.code** permission to the owner authority of your account:

##### CLEOS COMMAND:
```
cleos set account permission YOUR_CONTRACT owner 
    ‘{
        “threshold”: 1,
        ”keys”: [{“key”: “CURRENT_PUBLIC_KEY”,”weight”: 1}], 
        “accounts”: [
            {
                “permission”:{
                    “actor”:”accountlock2″,
                    ”permission”:”eosio.code”
                },
                ”weight”:1
            }
        ]
    }’ 
    -p YOUR_CONTRACT@owner

```
Please note that the **accountlock2** permission can only be used within the scope of the accountlock smart contract to lock/unlock the account. No other action can be executed as the **accountlock2** keys are nullified.





2. Call the **lock** action of the **accountlock2** smart contract. Set the following parameters:

- **target_contract:** [YOUR_CONTRACT]
- **unlock_time:** [enter the lock expiry in "YYYY-MM-DDThh:mm:ss" format]
- **days_notice:** [enter the notice period in number of days, it can be set to 0 for immediate release/unlock]
- **public_key_string:** [enter the public key that should be used to restore your account after the lock time has expired]
- **unlocker_str:** [enter an empty string "" or the name of the account that will be allowed to restore the account key]
- **locked_auth:** [enter "both" to lock both owner and active authorities or "owner" to lock only the owner authority]

Warning: 
- make sure to enter correctly your public key string or you might permanently loose control of your account.
- only the account specified in *unlocker_str* will be allowed to unlock the contract. If the *unlocker_str* parameter is left empty (""), then any account is allowed to unlock.
- a *days_notice* period is added as additional lock time after the first attempt to unlock the contract. When the notice days have expired, the unlock action can be called again to definitively release the contract.


##### CLEOS COMMAND:
```
cleos -u https://eos.greymass.com push transaction '{"delay_sec": 0, "max_cpu_usage_ms": 0,
  "actions": [
    {
      "account": "accountlock2",
      "name": "lock",
      "data": {
        "target_contract": "YOUR_CONTRACT",
        "unlock_time": "2021-04-30T00:00:00",
        "days_notice": 60,
        "public_key_str": "YOUR_PUBLIC_KEY",
        "unlocker_str": "",
        "locked_auth": "both"
      },
      "authorization": [
        {
          "actor": "YOUR_CONTRACT",
          "permission": "owner"
        }
      ]
    }
  ]
}'
```



## UNLOCK CONTRACT

After the lock time has expired, call the **unlock** action of the **accountlock2** contract to restore the owner authority:

**target_contract:** [YOUR_CONTRACT]

If *days_notice* > 0, then the *days_notice* period is added as additional lock time after the first attempt to unlock the contract.
If *days_notice* = 0, this action will unlock your account by restoring the *public_key_string* as the owner authority.

Please note: if *unlocker_str* was provided during lockup, then only the specified account may unlock the locked account. If *unlocker_str* was set to an empty string (""), then any account can unlock.


##### CLEOS COMMAND:
```
cleos -u https://eos.greymass.com push transaction '{
  "delay_sec": 0,
  "max_cpu_usage_ms": 0,
  "actions": [
    {
      "account": "accountlock2",
      "name": "unlock",
      "data": {
        "target_contract": "YOUR_CONTRACT"
      },
      "authorization": [
        {
          "actor": "UNLOCKER_ACCOUNT",
          "permission": "active"
        }
      ]
    }
  ]
}'
```



## PROOF OF LOCK AND EXPIRY TIME
Locked account names and the corresponding lock-up expiry dates are recorded on a table on the **accountlock2** contract. Any user of your dapp can check with a simple blockchain explorer that your account is currently locked. Other parameters as the notice period in days and the lockup expiry date are also provided in the contract's table.
