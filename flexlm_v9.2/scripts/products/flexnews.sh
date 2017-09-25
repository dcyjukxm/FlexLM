EXPIRE_DATE=$1
START=`pwd`
cd /u/gsihome/site

if [ "$#" -ne 1 ]
then
	echo "Need to supply expiration date."
	echo "Usage: sh name_passwd yyyy-mm-dd"
	exit 1
fi


echo 'step 1 -- contracts from express_maint_contracts which expire >=' $EXPIRE_DATE
./gtl -sql "select contract_num from express_maint_contracts where expiration_date >= \"`echo $EXPIRE_DATE`\"" | sed '1,2d' | sed 's/$/", \\'/ |sed 's/^/"/'> /tmp/maillist.1

echo 'step 2 -- maint-num from order_items where contracts == step1 output'
./gtl -sql "select maintenance_con from order_items where product = \"FLEXlm\" and type = \"prod\" and maintenance_con = ANY( `cat /tmp/maillist.1`
\"nosuch\" )" | sed  '1,2d' | sed '/^$/d' | sed 's/$/\", \\/' | sed 's/^/\"/'> /tmp/maillist.2

echo 'step 3 -- company-id from supportContacts where main-num == step2 output'
./gtl -sql "select siteId from supportContacts where maintNum = ANY(`cat /tmp/maillist.2`
\"nosuch\" )"  | sed  '1,2d' | sed '/^$/d' | sed 's/$/\", \\/' | sed 's/^/\"/'> /tmp/maillist.3

echo 'step 3a -- person-id from supportContacts where (same as step3)'
./gtl -sql "select personId from supportContacts where maintNum = \
	ANY(`cat /tmp/maillist.2`
	\"nosuch\" )"  | sed  '1,2d' | sed '/^$/d' | sed 's/$/\", \\/' | 
	sed 's/^/\"/'> /tmp/maillist.3a

echo 'step 4 -- email from people where category flex and company == step3 output'
./gtl -sql "select email from people where \
		(category glob \"*tech_flex*\" or \
				category glob \"*contact_flex*\" or \
				category glob \"*addl_flex*\" or \
				category glob \"*alt_flex*\" or \
				category glob \"*tech_lmfree*\" or \
				category glob \"*contact_lmfree*\" or \
				category glob \"*dev_news*\" ) \
        and NOT(company_id->credit_status LIKE_IC \"%hold%\" ) \
	and company_id = ANY(`cat /tmp/maillist.3`
\"nosuch\" )"  |
sed  '1,2d' | sed '/^$/d' > /tmp/maillist.4

echo 'step 4a -- email from people where person_id == step 3a output'
./gtl -sql "select email from people where \
        NOT(company_id->credit_status LIKE_IC \"%hold%\" ) \
        and id = ANY(`cat /tmp/maillist.3a`
\"nosuch\" )"  |
sed  '1,2d' | sed '/^$/d' > /tmp/maillist.4a

cd $START

#common part of maillist is $mOUT
mOUT=maillist.`date '+%Y.%m.%d'`
cat /tmp/maillist.4 /tmp/maillist.4a | sort | uniq > $mOUT

#flex dev news maillist is $fOUT
fOUT=flexnews.`date '+%Y.%m.%d'`
echo "flexnews maillist in " $fOUT
cp $mOUT $fOUT

#add special cases to $fOUT
echo 'all_gsi@globes.com' >> $fOUT
echo 'masaok@globes.com' >> $fOUT
echo 'hassie@globes.com' >> $fOUT

rm /tmp/maillist.*
