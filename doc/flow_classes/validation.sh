for z in doc_h doc_c doc_i doc_o doc_also doc_f; do
  echo -e "\e[0;1;33;44m----$z----\e[0m"
  egrep -c "obj.*\<$z\>" *-help.pd | grep -v :1
done
