push:
	@git add .
	@git commit -m "update" || echo "Rien à commit"
	@git push