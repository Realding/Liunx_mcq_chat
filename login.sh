clear
echo "|*************************************************************************"
echo "|                    WELCOME USEING MCQ CHAT ROOM                        |"
echo "|------------------------------------------------------------------------|"
echo "|                                                                        |"
echo "|             Please choose the way you want to Login:                   |"
echo "|             >> 1 Login.                                                |"
echo "|             >> 2 Register.                                             |"
echo "|             >> 3 Login as guest.                                       |"
echo "|             >> Q Quit.                                                 |"
echo "|                                                                        |"
echo "*************************************************************************|"


while :
do
	echo "[Menu] Choose from ( 1, 2, 3, 'Q' )..."
	read -p ">>" choice

	case "$choice" in
		"1" )
			#read users from file
			s_user=()
			s_passwd=()
			s_nickname=()
			i=0
			while read line
			do
			    s_user[$i]=`echo $line | cut -d ":" -f 1`
				s_passwd[$i]=`echo $line | cut -d ":" -f 2`
				s_nickname[$i]=`echo $line | cut -d ":" -f 3`
				i=`expr $i + 1` #let i++
			done < mypasswd.txt
			echo "There is ${#s_user[*]} user's accounts."
			echo ${s_user[@]}

			#check login account
			echo "Please Input your account number:"
			read account
			if ! [[ "${s_user[@]}" =~ $account ]]
			then
				echo "[Error 1]: Account not exists."
				continue
			fi

			#the number
			num=0
			for v in ${s_user[*]}
			do
				if [ "$v" == "$account" ]
				then
					break
				fi
				num=`expr $num + 1`
			done
			#echo "num:${num}"
			
			#read the password
			echo "Please Input your password:"
			while : ;do
				#echo off
				char=`
					stty cbreak -echo
					dd if=/dev/tty bs=1 count=1 2>/dev/null
					stty -cbreak echo
				`
				if [ "$char" =  "" ];then
					echo   #print \n
					break
				fi
				password="$password$char"
				echo -n "*"
			done			

			echo $password
			#read -s password
			if ! [[ "${s_passwd[num]}" == $password ]]
			then
				echo "[Error 2]: Password incorrect."
				password=""
				continue
			fi
			nickname=${s_nickname[num]}
			echo "Welcome ${nickname},you have enter the chat room!"
			break
			;;
		"2" )
			#check account 
			s_user=()
			i=0
			while read line
			do
			    s_user[$i]=`echo $line | cut -d ":" -f 1`
				i=`expr $i + 1` #let i++
			done < mypasswd.txt


			echo "Please Input the account number you want to register:"
			read newaccount
			if [[ "${s_user[@]}" =~ $newaccount ]]
			then
				echo "[Error 3]: This account has been registered."
				continue
			fi
			echo "Please Input your password:"
			read -s newpassword
			echo "Please Input your password again:"
			read -s checkpassword
			if [ $checkpassword == $newpassword ]
			then
				read -p "Input your nickname:" nickname
				echo "Register successed!"
				#save register information
				echo "${newaccount}:${newpassword}:${nickname}" >> mypasswd.txt
			else
				echo "[Error 4]: Register Failed! The password you input is not the same."
			fi

			;;
		"3" )
			read -p "Input your nickname:" nickname
			nickname="Guest_${nickname}" 
			echo "Welcome ${nickname},you have enter the chat room!"
			./mcq_client localhost "$nickname"
			break
			;;
		"Q" | "q" )
			echo "Bye"
			break
			;;
		* )
			;;
	esac
done
