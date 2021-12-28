let s:cpo_save=&cpo
set cpo&vim
cnoremap <silent> <S-Tab> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591871)
cnoremap <silent> <C-Space> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591865)
inoremap <silent> <C-Space> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591864)
cnoremap <silent> <Down> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591863)
inoremap <silent> <Down> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591862)
cnoremap <silent> <Up> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591857)
inoremap <silent> <Up> <Cmd>call v:lua.cmp.utils.keymap.set_map(31591856)
nnoremap <silent>  :nohl
vnoremap  :'<,'>!sh -c "$(head -n1)"
nnoremap <silent>  o :SymbolsOutline
omap <silent> % <Plug>(MatchitOperationForward)
xmap <silent> % <Plug>(MatchitVisualForward)
nmap <silent> % <Plug>(MatchitNormalForward)
nnoremap Y y$
omap <silent> [% <Plug>(MatchitOperationMultiBackward)
xmap <silent> [% <Plug>(MatchitVisualMultiBackward)
nmap <silent> [% <Plug>(MatchitNormalMultiBackward)
nnoremap \db :GdbStartBashDB bashdb main.sh
nnoremap \dp :GdbStartPDB python -m pdb main.py
nnoremap \dl :GdbStartLLDB lldb
nnoremap \dd :GdbStart gdb -q 
omap <silent> ]% <Plug>(MatchitOperationMultiForward)
xmap <silent> ]% <Plug>(MatchitVisualMultiForward)
nmap <silent> ]% <Plug>(MatchitNormalMultiForward)
xmap a% <Plug>(MatchitVisualTextObject)
xmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
omap <silent> g% <Plug>(MatchitOperationBackward)
xmap <silent> g% <Plug>(MatchitVisualBackward)
nmap <silent> g% <Plug>(MatchitNormalBackward)
xnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(netrw#GX(),netrw#CheckIfRemote(netrw#GX()))
xmap <silent> <Plug>(MatchitVisualTextObject) <Plug>(MatchitVisualMultiBackward)o<Plug>(MatchitVisualMultiForward)
onoremap <silent> <Plug>(MatchitOperationMultiForward) :call matchit#MultiMatch("W",  "o")
onoremap <silent> <Plug>(MatchitOperationMultiBackward) :call matchit#MultiMatch("bW", "o")
xnoremap <silent> <Plug>(MatchitVisualMultiForward) :call matchit#MultiMatch("W",  "n")m'gv``
xnoremap <silent> <Plug>(MatchitVisualMultiBackward) :call matchit#MultiMatch("bW", "n")m'gv``
nnoremap <silent> <Plug>(MatchitNormalMultiForward) :call matchit#MultiMatch("W",  "n")
nnoremap <silent> <Plug>(MatchitNormalMultiBackward) :call matchit#MultiMatch("bW", "n")
onoremap <silent> <Plug>(MatchitOperationBackward) :call matchit#Match_wrapper('',0,'o')
onoremap <silent> <Plug>(MatchitOperationForward) :call matchit#Match_wrapper('',1,'o')
xnoremap <silent> <Plug>(MatchitVisualBackward) :call matchit#Match_wrapper('',0,'v')m'gv``
xnoremap <silent> <Plug>(MatchitVisualForward) :call matchit#Match_wrapper('',1,'v')m'gv``
nnoremap <silent> <Plug>(MatchitNormalBackward) :call matchit#Match_wrapper('',0,'n')
nnoremap <silent> <Plug>(MatchitNormalForward) :call matchit#Match_wrapper('',1,'n')
tnoremap <silent> <Plug>(fzf-normal) 
tnoremap <silent> <Plug>(fzf-insert) i
nnoremap <silent> <Plug>(fzf-normal) <Nop>
nnoremap <silent> <Plug>(fzf-insert) i
snoremap <expr> <BS> ("\<BS>" . (getcurpos()[2] == col('$') - 1 ? 'a' : 'i'))
tmap <Plug>(cmp.utils.keymap.normalize) 
cnoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591873)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591872)
cnoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591868)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591867)
cnoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591870)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591869)
cnoremap <silent> 	 <Cmd>call v:lua.cmp.utils.keymap.set_map(31591866)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591874)
cnoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591861)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591860)
cnoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591859)
inoremap <silent>  <Cmd>call v:lua.cmp.utils.keymap.set_map(31591858)
inoremap  u
inoremap  u
let &cpo=s:cpo_save
unlet s:cpo_save
set completeopt=menu,menuone,noselect
set helplang=en
set listchars=tab:¸\ ,trail:·
set runtimepath=~/.config/nvim,~/.config/nvim/plugged/nvim-lspconfig,~/.config/nvim/plugged/cmp-nvim-lsp,~/.config/nvim/plugged/cmp-buffer,~/.config/nvim/plugged/cmp-path,~/.config/nvim/plugged/cmp-cmdline,~/.config/nvim/plugged/nvim-cmp,~/.config/nvim/plugged/cmp-vsnip,~/.config/nvim/plugged/vim-vsnip,~/.config/nvim/plugged/vim-templates,~/.config/nvim/plugged/vim-airline,~/.config/nvim/plugged/vim-airline-themes,~/.config/nvim/plugged/nvim-gdb,~/.config/nvim/plugged/popfix,~/.config/nvim/plugged/nvim-lsputils,~/.config/nvim/plugged/editorconfig-vim,~/.config/nvim/plugged/symbols-outline.nvim,~/.config/nvim/plugged/copilot.vim,/etc/xdg/nvim,~/.local/share/nvim/site,~/.local/share/flatpak/exports/share/nvim/site,/var/lib/flatpak/exports/share/nvim/site,/usr/local/share/nvim/site,/usr/share/nvim/site,/usr/share/nvim/runtime,/usr/share/nvim/runtime/pack/dist/opt/matchit,/usr/lib/nvim,/usr/share/nvim/site/after,/usr/local/share/nvim/site/after,/var/lib/flatpak/exports/share/nvim/site/after,~/.local/share/flatpak/exports/share/nvim/site/after,~/.local/share/nvim/site/after,/etc/xdg/nvim/after,~/.config/nvim/plugged/cmp-nvim-lsp/after,~/.config/nvim/plugged/cmp-buffer/after,~/.config/nvim/plugged/cmp-path/after,~/.config/nvim/plugged/cmp-cmdline/after,~/.config/nvim/plugged/cmp-vsnip/after,~/.config/nvim/after
set shiftwidth=2
set softtabstop=2
set tabstop=2
set window=42
" vim: set ft=vim :
