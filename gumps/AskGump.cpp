/*
 *  Copyright (C) 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "pent_include.h"
#include "AskGump.h"
#include "ButtonWidget.h"
#include "UCList.h"
#include "UCMachine.h"
#include "IDataSource.h"
#include "ODataSource.h"


// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(AskGump,ItemRelativeGump);

AskGump::AskGump()
	: ItemRelativeGump(), answers(0)
{

}

AskGump::AskGump(uint16 owner, UCList *answers_) : 
	ItemRelativeGump(0, 0, 0, 0, owner, 0, LAYER_ABOVE_NORMAL), answers(new UCList(2))
{
	answers->copyStringList(*answers_);
}

AskGump::~AskGump()
{
	answers->freeStrings();
	delete answers;
}

// Init the gump, call after construction
void AskGump::InitGump()
{
	ItemRelativeGump::InitGump();

	// OK, this is a bit of a hack, but it's how it's has to be
	int	fontnum;
	if (owner == 1) fontnum = 6;
	else if (owner > 256) fontnum = 8;
	else switch (owner%3) {
		case 1:
			fontnum = 5;
			break;
			
		case 2:
			fontnum = 7;
			break;

		default:
			fontnum = 0;
			break;
	}

	int px = 0, py = 0;

	for (unsigned int i = 0; i < answers->getSize(); ++i) {
		pout << i << ": " << UCMachine::get_instance()->getString(answers->getStringIndex(i)) << std::endl;
		std::string str_answer = "@ ";
		str_answer += UCMachine::get_instance()->getString(answers->getStringIndex(i));

		Gump *child = new ButtonWidget(px, py, str_answer, fontnum);
		child->InitGump();
		child->SetIndex(i);
		AddChild(child);

		Pentagram::Rect cd;
		child->GetDims(cd);

		if (px+cd.w > 160 && px != 0) 
		{
			py = dims.h;
			px = 0;
			child->Move(px,py);
		}

		if (cd.w+px > dims.w) dims.w = cd.w+px;
		if (cd.h+py > dims.h) dims.h = cd.h+py;

		px += cd.w+4;
	}
}

void AskGump::ChildNotify(Gump *child, uint32 message)
{
	if (message == ButtonWidget::BUTTON_DOWN) 
	{
		uint16 s = answers->getStringIndex(child->GetIndex());
		process_result = s;

		// answers' strings are going to be deleted, so make sure
		// the response string won't be deleted
		answers->removeString(s, true); //!! assuming that answers doesn't
                                        //!! contain two identical strings
		Close();
	}
}

void AskGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	ItemRelativeGump::saveData(ods);

	answers->save(ods);
}

bool AskGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!ItemRelativeGump::loadData(ids)) return false;

	answers = new UCList(2);
	answers->load(ids);

	return true;
}

// You should always use Protection
