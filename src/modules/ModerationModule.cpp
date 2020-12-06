#include "ModerationModule.h"
#include "UmikoBot.h"
#include "Core/Permissions.h"

using namespace Discord;

ModerationModule::ModerationModule()
	: Module("moderation", true)
{
	RegisterCommand(Commands::MODERATION_INVITATION_TOGGLE, "invitations",
		[this](Client& client, const Message& message, const Channel& channel)
	{
		m_invitationModeration ^= true;
		client.createMessage(message.channelId(), m_invitationModeration ? "Invitations will be deleted!" : "Invitations won't be deleted!");
	});
}

void ModerationModule::OnMessage(Client& client, const Message& message)
{
	client.getChannel(message.channelId()).then(
	[this, message, &client](const Channel& channel)
	{
		if (m_invitationModeration)
		{
			if (message.content().contains("https://discord.gg/", Qt::CaseInsensitive))
			{
				auto authorID = message.author().id();
				::Permissions::ContainsPermission(client, channel.guildId(), message.author().id(), CommandPermission::ADMIN,
					[this, message, &client, authorID](bool result)
				{
					if (!result)
					{
						client.deleteMessage(message.channelId(), message.id());
						client.createMessage(message.channelId(), "**<@" + QString::number(authorID) + ">, you can't publicly advertise servers!**");
					}
				});
			}
		}
	});

	Module::OnMessage(client, message);
}


void ModerationModule::OnSave(QJsonDocument& doc) const 
{
	QJsonObject json;
	QJsonObject moderation;

	moderation["invitationModeration"] = m_invitationModeration;
	json["moderation"] = moderation;

	doc.setObject(moderation);
}

void ModerationModule::OnLoad(const QJsonDocument& doc)
{
	QJsonObject json = doc.object();
	QJsonObject moderation = json["moderation"].toObject();
	m_invitationModeration = moderation["moderation"].toBool();
}
