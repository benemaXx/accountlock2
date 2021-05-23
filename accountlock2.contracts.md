<h1 class="contract">lock</h1>

---

spec-version: 2.0.0
title: lock
summary: 'Locks an account for a given period of time by temporarily removing its permissions keys'
icon:

---

Requires the account's owner authority.

{{$action.account}} agrees to replace {{locked_auth}} permission/s keys for the account {{target_contract}} with the accountlock2@eosio.code permission.

{{$action.account}} acknowledges that the permission keys can be restored only after the unlock date/time which is set on {{unlock_time}} and only after a notice period of {{days_notice}} days has elapsed since the first call to the "unlock" action.

Moreover, {{$action.account}} acknowledges that the permission keys can be restored only by the following account: {{unlocker_str}} (if not specified, any account can restore keys after {{unlock_time}} and notice has elapsed).

{{$action.account}} specifies the following key as the public key to be restored when the account will be unlocked: {{public_key_str}}



<h1 class="contract">unlock</h1>

---

spec-version: 2.0.0
title: unlock
summary: 'unlocks the account by restoring the public key provided during lockup or starts the notice period'
icon:

---

Requires authority of the unlocker account or no authority if no unlocker account was provided during lockup.

If a notice period is required, {{$action.account}} agrees to start the notice period after which the account {{target_contract}} can be unlocked by a new call to this unlock action.

If a notice period is not required or has elapsed, {{$action.account}} agrees to restore the permission keys for the account {{target_contract}} using the public key provided during lockup.



