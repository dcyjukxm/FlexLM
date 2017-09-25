Instructions for Sending Out the monthly User Name and Password

1) Prepare the User Name and Password text, place it in:

	~pattyh/Flexnews/namepasswd.yymm

   where "yymm" is the year and month for which the
   user name and password apply.


2) Generate list of email addresses:

	rlogin to birchlake (for its Java setup)
	cd ~pattyh/Workarea/scripts/products
	sh namepasswd.sh yyyy-mm-dd

   The following is output from the script:
  
        step 1 -- contracts from express_maint_contracts which expire >= 2002-11-01
        step 2 -- maint-num from order_items where contracts == step1 output
        step 3 -- company-id from supportContacts where main-num == step2 output
        step 3a -- person-id from supportContacts where (same as step3)
        step 4 -- email from people where category flex and company == step3 output
        step 4a -- email from people where person_id == step 3a output
        name and password maillist in  namepasswd.2002.10.29

    
    The output file, namepasswd.2002.10.29 in this example, should have 1000's 
    of email addresses.


3) Prepare the list of contacts:

	cp namepasswd.2002.10.29 ~pattyh/Flexnews/maillist
	cd ~pattyh/Flexnews (it has to be this specific directory)
	chmod 644 maillist


4) Email the list out:

	rlogin to wheel as user "infomail" (passwd = gsi4infomail)
	cd ~pattyh/Flexnews
	mailx -b gsi_list -s"FLEXlm User Name and Password - November 2002" \
              FLEXlm_developers < namepasswd.0211
	
   "gsi_list" is an email alias which grabs the contents of
   "maillist".  This alias is owned and maintained by sysadmin and is
   defined as:
          :include:/u2/pattyh/Flexnews/maillist


5) Let Loree know that this has been sent
   out.  She could receive several bounces into the infomail
   box for obsolete email addresses.


6) Afer an hour or so, the delivery should be complete. To confirm, 
   check your email inbox for your personal copy.




	


