clear
echo "|*************************************************************"
echo "|                                                            |"
echo "|                    欢迎使用MCQ聊天工具                     |"
echo "|                                                            |"
echo "|------------------------------------------------------------|"
echo "|                                                            |"
echo "|                                                            |"
echo "|             请选择登录方式：                               |"
echo "|                                                            |"
echo "|             1 用户名登录.                                  |"
echo "|             2 注册.                                        |"
echo "|             3 游客登录.                                    |"
echo "|             Q 退出.                                        |"
echo "|                                                            |"
echo "|                                                            |"
echo "*************************************************************|"


while :
do
	time=$(date "+%y/%m/%d %H:%M:%S")
	echo
	echo "[系统时间:${time}]请选择登录方式："
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
			#echo "There is ${#s_user[*]} user's accounts."
			#echo ${s_user[@]}

			#check login account
			echo "请输入帐号："
			read account
			if ! [[ "${s_user[@]}" =~ $account ]]
			then
				echo "[Error 1]: 帐号不存在."
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
			echo "请输入密码："
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
				echo "[Error 2]: 密码不正确."
				password=""
				continue
			fi
			nickname=${s_nickname[num]}
			echo "验证成功! ${nickname} 欢迎您,正在进入聊天室……"
			./mcq_client localhost "$nickname"
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


			echo "请输入注册帐号："
			read newaccount
			if [[ "${s_user[@]}" =~ $newaccount ]]
			then
				echo "[Error 3]: 该账号已被注册."
				continue
			fi
			echo "请输入密码："
			read -s newpassword
			echo "请再次输入密码："
			read -s checkpassword
			if [ $checkpassword == $newpassword ]
			then
				read -p "请输入您的昵称：" nickname
				echo "注册成功!"
				#save register information
				echo "${newaccount}:${newpassword}:${nickname}" >> mypasswd.txt
			else
				echo "[Error 4]: 注册失败，两次输入的密码不一致."
			fi

			;;
		"3" )
			read -p "请输入昵称：" nickname
			nickname="_${nickname}" 
			echo "${nickname} 您好,正在进入聊天室……"
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
