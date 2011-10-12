case "$1" in
     js)
			./build/mediatomb -D -c scripts/config.xml.test.js -a ~/Videos/
           ;;
     py)
			./build/mediatomb -D -c scripts/config.xml.test -a ~/Videos/
           ;;
	 info)
			./build/mediatomb --compile-info
			;;
     *)
           echo "py run with Python Runtime"
           echo "js run with SpiderMonkey Runtime"
           echo "Compile-Info"
           exit
           ;;
esac        







