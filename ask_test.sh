if ./ask.sh "Do you want to continue?" Y; then
  echo "You want to continue!"
else
  echo "You don't want to continue"
fi

if ./ask.sh "Do you want to continue?"; then
  echo "You want to continue!"
else
  echo "You don't want to continue"
fi